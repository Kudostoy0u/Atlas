import type { DemoDocument, IndexStats, SearchResult } from "./types";

type Posting = {
  docId: number;
  tf: number;
};

type EncodedTerm = {
  docBytes: number;
  tfBytes: number;
};

const encoder = new TextEncoder();

function tokenize(text: string): string[] {
  return text
    .toLowerCase()
    .split(/[^a-z0-9]+/g)
    .filter(Boolean);
}

function varintSize(value: number): number {
  let size = 1;
  while (value >= 0x80) {
    value >>>= 7;
    size += 1;
  }
  return size;
}

function snippetFor(document: DemoDocument, queryTerms: string[]) {
  const words = document.body.split(/\s+/);
  const firstHit = words.findIndex((word) =>
    queryTerms.some((term) => word.toLowerCase().includes(term))
  );
  const start = Math.max(0, firstHit === -1 ? 0 : firstHit - 8);
  return words.slice(start, start + 26).join(" ");
}

export class BrowserAtlasIndex {
  private documents: DemoDocument[] = [];
  private lengths: Uint32Array = new Uint32Array();
  private postings = new Map<string, Posting[]>();
  private encoded = new Map<string, EncodedTerm>();
  private avgDocumentLength = 0;
  private totalTokens = 0;
  private rawBytes = 0;
  private compressedBytes = 0;

  build(documents: DemoDocument[]): IndexStats {
    const started = performance.now();
    this.documents = documents;
    this.lengths = new Uint32Array(documents.length);
    this.postings = new Map();
    this.encoded = new Map();
    this.totalTokens = 0;
    this.rawBytes = 0;

    for (const document of documents) {
      const frequencies = new Map<string, number>();
      const tokens = tokenize(`${document.title} ${document.department} ${document.body}`);
      this.lengths[document.id] = tokens.length;
      this.totalTokens += tokens.length;
      this.rawBytes += encoder.encode(
        `${document.externalId}\t${document.title}\t${document.department}\t${document.body}`
      ).byteLength;

      for (const token of tokens) {
        frequencies.set(token, (frequencies.get(token) ?? 0) + 1);
      }

      for (const [term, tf] of frequencies) {
        const list = this.postings.get(term) ?? [];
        list.push({ docId: document.id, tf });
        this.postings.set(term, list);
      }
    }

    this.avgDocumentLength = this.totalTokens / Math.max(1, documents.length);
    this.compressedBytes = this.estimateCompressedBytes();
    const indexMs = performance.now() - started;

    return {
      documents: documents.length,
      terms: this.postings.size,
      tokens: this.totalTokens,
      rawBytes: this.rawBytes,
      compressedBytes: this.compressedBytes,
      indexMs,
      throughputMiBs: this.rawBytes / 1024 / 1024 / (indexMs / 1000),
      compressionReduction: 1 - this.compressedBytes / this.rawBytes
    };
  }

  search(query: string, limit: number): SearchResult[] {
    const queryTerms = tokenize(query);
    const scores = new Map<number, number>();
    const k1 = 1.2;
    const b = 0.75;

    for (const term of queryTerms) {
      const postings = this.postings.get(term);
      if (!postings) {
        continue;
      }

      const idf = Math.log(1 + (this.documents.length - postings.length + 0.5) / (postings.length + 0.5));
      for (const posting of postings) {
        const dl = this.lengths[posting.docId];
        const numerator = posting.tf * (k1 + 1);
        const denominator = posting.tf + k1 * (1 - b + b * (dl / this.avgDocumentLength));
        scores.set(posting.docId, (scores.get(posting.docId) ?? 0) + idf * (numerator / denominator));
      }
    }

    return [...scores.entries()]
      .sort((a, bScore) => bScore[1] - a[1] || a[0] - bScore[0])
      .slice(0, limit)
      .map(([id, score]) => {
        const document = this.documents[id];
        return {
          id,
          externalId: document.externalId,
          title: document.title,
          department: document.department,
          jurisdiction: document.jurisdiction,
          year: document.year,
          pages: document.pages,
          score,
          snippet: snippetFor(document, queryTerms),
          body: document.body
        };
      });
  }

  private estimateCompressedBytes() {
    let bytes = 0;

    for (const [term, postings] of this.postings) {
      let previous = 0;
      let docBytes = 0;
      let tfBytes = 0;
      for (const posting of postings) {
        docBytes += varintSize(posting.docId - previous);
        tfBytes += varintSize(posting.tf);
        previous = posting.docId;
      }
      this.encoded.set(term, { docBytes, tfBytes });
      bytes += encoder.encode(term).byteLength + docBytes + tfBytes + 8;
    }

    for (const document of this.documents) {
      bytes += encoder.encode(`${document.externalId}${document.title}`).byteLength + 8;
    }

    return bytes;
  }
}

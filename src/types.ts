export type DemoDocument = {
  id: number;
  externalId: string;
  title: string;
  department: string;
  jurisdiction: string;
  year: number;
  pages: number;
  body: string;
};

export type SearchResult = {
  id: number;
  externalId: string;
  title: string;
  department: string;
  jurisdiction: string;
  year: number;
  pages: number;
  score: number;
  snippet: string;
  body: string;
};

export type IndexStats = {
  documents: number;
  terms: number;
  tokens: number;
  rawBytes: number;
  compressedBytes: number;
  indexMs: number;
  throughputMiBs: number;
  compressionReduction: number;
};

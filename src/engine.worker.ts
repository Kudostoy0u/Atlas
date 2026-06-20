import { generateDocuments } from "./corpus";
import { BrowserAtlasIndex } from "./searchEngine";

const index = new BrowserAtlasIndex();

self.onmessage = (event: MessageEvent) => {
  const { type, payload, requestId } = event.data;

  if (type === "build") {
    const documents = generateDocuments(payload.count);
    const stats = index.build(documents);
    self.postMessage({ type: "built", requestId, stats });
    return;
  }

  if (type === "search") {
    const started = performance.now();
    const results = index.search(payload.query, payload.limit);
    self.postMessage({
      type: "results",
      requestId,
      results,
      latencyMs: performance.now() - started
    });
  }
};

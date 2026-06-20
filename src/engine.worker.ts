const apiBase = "http://127.0.0.1:8787";

self.onmessage = async (event: MessageEvent) => {
  const { type, payload, requestId } = event.data;

  if (type === "build") {
    const response = await fetch(`${apiBase}/api/build?count=${payload.count}`);
    const stats = await response.json();
    self.postMessage({ type: "built", requestId, stats });
    return;
  }

  if (type === "search") {
    const params = new URLSearchParams({
      q: payload.query,
      limit: String(payload.limit)
    });
    const response = await fetch(`${apiBase}/api/search?${params}`);
    const { results, latencyMs } = await response.json();
    self.postMessage({
      type: "results",
      requestId,
      results,
      latencyMs
    });
  }
};

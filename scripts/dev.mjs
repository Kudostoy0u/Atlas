import { spawn } from "node:child_process";
import { existsSync } from "node:fs";

const sources = [
  "src/compression.cpp",
  "src/document.cpp",
  "src/tokenizer.cpp",
  "src/index.cpp",
  "src/index_builder.cpp",
  "apps/demo_server.cpp"
];

function run(command, args, options = {}) {
  return new Promise((resolve, reject) => {
    const child = spawn(command, args, { stdio: "inherit", ...options });
    child.on("exit", (code) => {
      if (code === 0) {
        resolve();
      } else {
        reject(new Error(`${command} exited with ${code}`));
      }
    });
  });
}

if (!existsSync("data/lexicon.txt")) {
  await run("npm", ["run", "build:lexicon"]);
}

await run("clang++", [
  "-std=c++20",
  "-O3",
  "-DNDEBUG",
  "-pthread",
  "-Iinclude",
  ...sources,
  "-o",
  "/tmp/atlas-demo-server"
]);

const server = spawn("/tmp/atlas-demo-server", ["data/lexicon.txt"], {
  stdio: "inherit"
});

const vite = spawn("vite", ["--host", "127.0.0.1", "--port", "5173"], {
  stdio: "inherit",
  shell: true
});

function shutdown() {
  server.kill("SIGTERM");
  vite.kill("SIGTERM");
}

process.on("SIGINT", shutdown);
process.on("SIGTERM", shutdown);

vite.on("exit", (code) => {
  server.kill("SIGTERM");
  process.exit(code ?? 0);
});

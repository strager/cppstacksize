#!/usr/bin/env node

import fs from "node:fs";
import http from "node:http";
import path from "node:path";
import url from "node:url";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

let port = 8000;
let baseURI = `http://localhost:${port}`;
let rootPath = path.join(__dirname, "..", "src");

let server = http.createServer((req, res) => {
  console.log(`${req.method} ${req.url}`);
  if (req.method !== "GET") {
    notFound();
  }

  let relativePath = new URL(req.url, baseURI).pathname;
  if (relativePath === "/") {
    relativePath = "/index.html";
  }
  if (isInsecurePath(relativePath)) {
    notFound();
    return;
  }

  let fullPath = path.join(rootPath, relativePath);
  let stream = fs.createReadStream(fullPath);
  stream.on("open", () => {
    res.writeHead(200, { "content-type": guessContentType(fullPath) });
  });
  stream.on("error", (err) => {
    if (err.code === "ENOENT") {
      notFound();
    } else {
      console.error(err);
      res.writeHead(500);
      res.write(err.toString());
      res.end();
    }
  });
  stream.pipe(res);

  function notFound() {
    res.writeHead(404, { "content-type": "text/plain" });
    res.end("404 Not Found");
  }
});
server.on("clientError", (err, socket) => {
  console.error(err);
  socket.end("HTTP/1.1 400 Bad Request\r\n\r\n");
});
server.on("error", (err) => {
  console.error(err);
});
server.listen(port);
console.log(`listening on ${baseURI}`);

function isInsecurePath(p) {
  if (p.includes("..") || p.includes(":")) {
    // Possible sandbox escape.
    return true;
  }
  if (!/^[a-zA-Z./_-]+$/.test(p)) {
    // Unexpected characters.
    return true;
  }
  return false;
}

function guessContentType(p) {
  if (/\.mjs$/.test(p)) {
    return "application/javascript";
  }
  if (/\.html$/.test(p)) {
    return "text/html";
  }
  if (/\.wasm$/.test(p)) {
    return "application/wasm";
  }
  return "text/plain";
}

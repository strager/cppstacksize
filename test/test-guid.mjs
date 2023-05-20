import assert from "node:assert";
import { GUID } from "../src/guid.mjs";
import { describe, it } from "node:test";

describe("GUID", (t) => {
  it("parses binary then stringifies", () => {
    let bytes = new Uint8Array([
      0x8d, 0x05, 0x7c, 0x59, 0xfe, 0xaf, 0xbf, 0x4a, 0xa0, 0xea, 0x76, 0xa2,
      0xe3, 0xa3, 0xd0, 0x99,
    ]);
    let guid = new GUID(bytes);
    assert.strictEqual(guid.toString(), "597c058d-affe-4abf-a0ea-76a2e3a3d099");
  });

  it("string includes 0 padding", () => {
    let bytes = new Uint8Array([
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    ]);
    let guid = new GUID(bytes);
    assert.strictEqual(guid.toString(), "00000000-0000-0000-0000-000000000000");
  });
});

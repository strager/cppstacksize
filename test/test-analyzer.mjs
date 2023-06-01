import assert from "node:assert";
import { Analyzer } from "../src/asm-stack-map.mjs";
import { describe, it } from "node:test";

describe("Analyzer", (t) => {
  it("gives stack data for simple x86_64 code", async () => {
    let analyzer = new Analyzer({ arch: "x86_64" });
    analyzer.setMachineCode(
      new Uint8Array([
        // mov %rax, 0x30(%rsp)
        0x48, 0x89, 0x44, 0x24, 0x30,
      ])
    );
    let stackMap = analyzer.getStackMapArraySlow();
    assert.deepStrictEqual(stackMap, [
      {
        entryRSPRelativeAddress: 0x30,
        byteCount: 8,
        instructionOffset: 0,
        isRead: false,
        isWrite: true,
      },
    ]);

    analyzer.dispose();
  });
});

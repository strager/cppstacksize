import assert from "node:assert";
import { describe, it } from "node:test";
import {
  assembleAsync,
  assembleTestASMsAsync,
} from "../src/update-test-asm.mjs";

describe("assemble", () => {
  it("should give bytes of one instruction", async () => {
    assert.deepStrictEqual(
      await assembleAsync(["xor %rax, %rax"], { arch: "x86_64" }),
      [0x48, 0x31, 0xc0]
    );

    // In yasm, this instruction's listing spans multiple lines.
    assert.deepStrictEqual(
      await assembleAsync(["movabsq $0x123456789abcdef0, %rbx"], {
        arch: "x86_64",
      }),
      [0x48, 0xbb, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12]
    );
  });

  it("should give bytes of multiple instructions", async () => {
    assert.deepStrictEqual(
      await assembleAsync(
        [
          "xor %eax, %eax",
          "xor %ebx, %ebx",
          "xor %ecx, %ecx",
          "xor %edx, %edx",
        ],
        { arch: "x86_64" }
      ),
      [
        0x31, 0xc0,
        //
        0x31, 0xdb,
        //
        0x31, 0xc9,
        //
        0x31, 0xd2,
      ]
    );
  });

  it("should give no bytes for labels", async () => {
    assert.deepStrictEqual(
      await assembleAsync([".loop:", "jmp .loop"], { arch: "x86_64" }),
      [0xeb, 0xfc]
    );
  });

  it("should give no bytes for blank lines", async () => {
    assert.deepStrictEqual(
      await assembleAsync(["", "xor %eax, %eax", "", ""], { arch: "x86_64" }),
      [0x31, 0xc0]
    );
  });
});

describe("update test ASM", () => {
  it("should do nothing with no directives", async () => {
    let source = "";
    let asms = await assembleTestASMsAsync(source);
    assert.deepStrictEqual(asms, new Map());
  });

  it("should make data for single x86_64 code line", async () => {
    let source =
      "ASM_X86_64(\n" + //
      '  "xor %eax, %eax"\n' + //
      ")\n";
    let asms = await assembleTestASMsAsync(source);
    assert.deepStrictEqual(asms, new Map([["xor %eax, %eax", [0x31, 0xc0]]]));
  });

  it("should make data for multiple x86_64 code lines", async () => {
    let source =
      "ASM_X86_64(\n" + //
      '  "xor %eax, %eax"\n' + //
      '  ".loop:"\n' + //
      '  "jmp .loop"\n' + //
      ")\n";
    let asms = await assembleTestASMsAsync(source);
    assert.deepStrictEqual(
      asms,
      new Map([["xor %eax, %eax.loop:jmp .loop", [0x31, 0xc0, 0xeb, 0xfc]]])
    );
  });

  it("should make data for ASM_X86_64 blocks independently", async () => {
    let source =
      "ASM_X86_64(\n" + //
      '  ".loop:"\n' + //
      '  "jmp .loop"\n' + //
      ")\n" + //
      "ASM_X86_64(\n" + //
      '  ".loop:"\n' + //
      '  "jne .loop"\n' + //
      ")\n";
    let asms = await assembleTestASMsAsync(source);
    assert.deepStrictEqual(
      asms,
      new Map([
        [".loop:jmp .loop", [0xeb, 0xfc]],
        [".loop:jne .loop", [0x75, 0xfc]],
      ])
    );
  });

  it("allows ')' inside strings", async () => {
    let source =
      "ASM_X86_64(\n" + //
      '  "mov (%rsp), %rbx"\n' + //
      ")\n";
    let asms = await assembleTestASMsAsync(source);
    assert.deepStrictEqual(
      asms,
      new Map([["mov (%rsp), %rbx", [0x48, 0x8b, 0x1c, 0x24]]])
    );
  });
});

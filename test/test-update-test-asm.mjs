import assert from "node:assert";
import { describe, it } from "node:test";
import { assembleAsync, updateTestASMAsync } from "../src/update-test-asm.mjs";

describe("assemble", () => {
  it("should give bytes of one instruction", async () => {
    assert.deepStrictEqual(
      await assembleAsync(["xor %rax, %rax"], { arch: "x86_64" }),
      [[0x48, 0x31, 0xc0]]
    );

    // In yasm, this instruction's listing spans multiple lines.
    assert.deepStrictEqual(
      await assembleAsync(["movabsq $0x123456789abcdef0, %rbx"], {
        arch: "x86_64",
      }),
      [[0x48, 0xbb, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12]]
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
        [0x31, 0xc0],
        [0x31, 0xdb],
        [0x31, 0xc9],
        [0x31, 0xd2],
      ]
    );
  });

  it("should give no bytes for labels", async () => {
    assert.deepStrictEqual(
      await assembleAsync([".loop:", "jmp .loop"], { arch: "x86_64" }),
      [
        [], // .loop
        [0xeb, 0xfc], // jmp .loop
      ]
    );
  });

  it("should give no bytes for blank lines", async () => {
    assert.deepStrictEqual(
      await assembleAsync(["", "xor %eax, %eax", "", ""], { arch: "x86_64" }),
      [[], [0x31, 0xc0], [], []]
    );
  });
});

describe("update test ASM", () => {
  it("should do nothing with no directives", async () => {
    let source = "";
    assert.strictEqual(await updateTestASMAsync(source), source);
  });

  it("should add bytes after single x86_64 code line", async () => {
    let source =
      "ASM_X86_64(\n" + //
      "  // xor %eax, %eax\n" + //
      ")\n";
    let expected =
      "ASM_X86_64(\n" + //
      "  // xor %eax, %eax\n" + //
      "  0x31, 0xc0,\n" + //
      ")\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("should add bytes after multiple x86_64 code lines", async () => {
    let source =
      "ASM_X86_64(\n" + //
      "  // xor %eax, %eax\n" + //
      "  // .loop:\n" + //
      "  // jmp .loop\n" + //
      ")\n";
    let expected =
      "ASM_X86_64(\n" + //
      "  // xor %eax, %eax\n" + //
      "  // .loop:\n" + //
      "  // jmp .loop\n" + //
      "  0x31, 0xc0,\n" + //
      "  0xeb, 0xfc,\n" + //
      ")\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("should replace bytes after single x86_64 code line", async () => {
    let source =
      "ASM_X86_64(\n" + //
      "  // xor %eax, %eax\n" + //
      "  0x69,\n" + //
      ")\n";
    let expected =
      "ASM_X86_64(\n" + //
      "  // xor %eax, %eax\n" + //
      "  0x31, 0xc0,\n" + //
      ")\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("should update ASM_X86_64 blocks independently", async () => {
    let source =
      "ASM_X86_64(\n" + //
      "  // .loop:\n" + //
      "  // jmp .loop\n" + //
      "  0x69,\n" + //
      ")\n" + //
      "ASM_X86_64(\n" + //
      "  // .loop:\n" + //
      "  // jne .loop\n" + //
      "  0x69,\n" + //
      ")\n";
    let expected =
      "ASM_X86_64(\n" + //
      "  // .loop:\n" + //
      "  // jmp .loop\n" + //
      "  0xeb, 0xfc,\n" + //
      ")\n" + //
      "ASM_X86_64(\n" + //
      "  // .loop:\n" + //
      "  // jne .loop\n" + //
      "  0x75, 0xfc,\n" + //
      ")\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("should preserve leading indentation after last byte", async () => {
    let source =
      "    ASM_X86_64(\n" + //
      "        // xor %eax, %eax\n" + //
      "    )\n";
    let expected =
      "    ASM_X86_64(\n" + //
      "        // xor %eax, %eax\n" + //
      "        0x31, 0xc0,\n" + //
      "    )\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });
});

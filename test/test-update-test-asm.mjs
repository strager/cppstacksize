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

  it("should add bytes before single x86_64 code line", async () => {
    let source =
      "// @asm-begin x86_64\n" + //
      "// @asm xor %eax, %eax\n" + //
      "// @asm-end\n";
    let expected =
      "// @asm-begin x86_64\n" + //
      "0x31, 0xc0,  // @asm xor %eax, %eax\n" + //
      "// @asm-end\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("should add bytes before multiple x86_64 code lines as needed", async () => {
    let source =
      "// @asm-begin x86_64\n" + //
      "// @asm xor %eax, %eax\n" + //
      "// @asm .loop:\n" + //
      "// @asm jmp .loop\n" + //
      "// @asm-end\n";
    let expected =
      "// @asm-begin x86_64\n" + //
      "0x31, 0xc0,  // @asm xor %eax, %eax\n" + //
      "// @asm .loop:\n" + //
      "0xeb, 0xfc,  // @asm jmp .loop\n" + //
      "// @asm-end\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("leaves non-@asm lines as-is", async () => {
    let source =
      "// @asm-begin x86_64\n" + //
      "hello\n" + //
      "// @asm xor %eax, %eax\n" + //
      "// world\n" + //
      "// @asm-end\n";
    let expected =
      "// @asm-begin x86_64\n" + //
      "hello\n" + //
      "0x31, 0xc0,  // @asm xor %eax, %eax\n" + //
      "// world\n" + //
      "// @asm-end\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("should replace bytes before single x86_64 code line", async () => {
    let source =
      "// @asm-begin x86_64\n" + //
      "0x69,  // @asm xor %eax, %eax\n" + //
      "// @asm-end\n";
    let expected =
      "// @asm-begin x86_64\n" + //
      "0x31, 0xc0,  // @asm xor %eax, %eax\n" + //
      "// @asm-end\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });

  it("aligns @asm comments", async () => {
    let source =
      "// @asm-begin x86_64\n" + //
      "0x31, 0xc0,  // @asm xor %eax, %eax\n" + //
      "// @asm mov %rbp, %rsp\n" + //
      "// don't touch this line\n" + //
      "// @asm hlt\n" + //
      "// @asm-end\n";
    let expected =
      "// @asm-begin x86_64\n" + //
      "0x31, 0xc0,        // @asm xor %eax, %eax\n" + //
      "0x48, 0x89, 0xec,  // @asm mov %rbp, %rsp\n" + //
      "// don't touch this line\n" + //
      "0xf4,              // @asm hlt\n" + //
      "// @asm-end\n";
    assert.strictEqual(await updateTestASMAsync(source), expected);
  });
});

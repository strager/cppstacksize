import child_process from "node:child_process";
import fs from "node:fs";
import { isModuleNodeMain } from "./is-main.mjs";

export async function updateTestASMAsync(source) {
  let asmDirectiveRE =
    /^(?<prefix>\s*?)(?<oldCode>\S.*?)?(?<comment>\/\/ (?<tag>@asm(?:-\S+)?)\s*(?<payload>.*))$/;
  let lines = source.split("\n");

  let outLines = [];

  // If assembleOptions is not null, we are between @asm-begin and @asm-end.
  let assembleOptions = null;
  let assemblyLines = [];
  let bufferedLinePrefixes = [];
  let bufferedLineSuffixes = [];

  for (let line of lines) {
    let match = line.match(asmDirectiveRE);
    if (match === null) {
      if (assembleOptions !== null) {
        assemblyLines.push("");
        bufferedLinePrefixes.push(line);
        bufferedLineSuffixes.push("");
      } else {
        outLines.push(line);
      }
      continue;
    }

    let payload = match.groups.payload;
    switch (match.groups.tag) {
      case "@asm-begin":
        if (assembleOptions !== null) {
          throw new Error(
            "unterminated @asm-begin; expected @asm-end before @asm-begin directive"
          );
        }
        assembleOptions = { arch: payload };
        assemblyLines.length = 0;
        bufferedLinePrefixes.length = 0;
        bufferedLineSuffixes.length = 0;
        assemblyLines.push("");
        bufferedLinePrefixes.push(line);
        bufferedLineSuffixes.push("");
        break;

      case "@asm-end": {
        assemblyLines.push("");
        bufferedLinePrefixes.push(line);
        bufferedLineSuffixes.push("");
        let instructionsBytes = await assembleAsync(
          assemblyLines,
          assembleOptions
        );
        let widthBeforeComment = 0;
        for (let i = 0; i < instructionsBytes.length; ++i) {
          let bytes = instructionsBytes[i];
          if (bytes.length > 0) {
            let prefix = bufferedLinePrefixes[i];
            let numbersLength = 6 * bytes.length -1;
            let thisLineWidthBeforeComment = prefix.length + numbersLength + 2;
            if (thisLineWidthBeforeComment > widthBeforeComment) {
              widthBeforeComment = thisLineWidthBeforeComment;
            }
          }
        }
        for (let i = 0; i < instructionsBytes.length; ++i) {
          let bytes = instructionsBytes[i];
          let prefix = bufferedLinePrefixes[i];
          let suffix = bufferedLineSuffixes[i];
          if (bytes.length > 0) {
            outLines.push(
              (prefix +
                makeCxxNumberLiterals(bytes)).padEnd(widthBeforeComment, ' ') +
                suffix
            );
          } else {
            outLines.push(
              prefix + suffix
            );
          }
        }
        assembleOptions = null;
        break;
      }

      case "@asm":
        assemblyLines.push(payload);
        bufferedLinePrefixes.push(match.groups.prefix);
        bufferedLineSuffixes.push(match.groups.comment);
        break;
    }
  }
  if (assembleOptions !== null) {
    throw new Error(
      "unterminated @asm-begin; expected @asm-end before end of file"
    );
  }

  return outLines.join("\n");
}

function makeCxxNumberLiterals(numbers) {
  return numbers.map((n) => `0x${n.toString(16).padStart(2, "0")}, `).join("").trim();
}

export async function assembleAsync(assemblyLines, { arch }) {
  console.assert(arch === "x86_64", "only x86_64 is supported right now");
  let listing = await getYASMListingAsync(
    ".code64\n" + assemblyLines.join("\n") + "\n"
  );
  let listingLineRE =
    /^ *(?<line>\d+) +((?<address>[0-9A-F]+) +(?<bytes>[0-9A-F]+)(?<toBeContinued>-?) +)?(?<code>.*)$/gm;

  let currentAssemblyLineIndex = 0;
  let instructions = [];
  let includeMatchInPreviousInstruction = false;
  for (let match of listing.matchAll(listingLineRE)) {
    if (includeMatchInPreviousInstruction) {
      instructions.at(-1).push(...hexStringToByteArray(match.groups.bytes));
    }
    includeMatchInPreviousInstruction = match.groups.toBeContinued === "-";

    while (assemblyLines[currentAssemblyLineIndex] === "") {
      instructions.push([]);
      currentAssemblyLineIndex += 1;
    }

    // TODO(strager): Handle leading whitespace in
    // assemblyLines[currentAssemblyLineIndex].
    if (assemblyLines[currentAssemblyLineIndex] === match.groups.code) {
      instructions.push(hexStringToByteArray(match.groups.bytes ?? ""));
      currentAssemblyLineIndex += 1;
    }
  }
  console.assert(currentAssemblyLineIndex === assemblyLines.length);
  return instructions;
}

function getYASMListingAsync(code) {
  return new Promise((resolve, reject) => {
    // HACK(strager): /dev/stdout does not work by default because Node.js's
    // 'pipe' option actually creates a UNIX socketpair, not a UNIX pipe. Give
    // yasm a UNIX pipe as stdout by piping to the 'cat' command.
    // HACK(strager): Because we pipe to 'cat', the shell will prefer 'cat''s
    // exit code and discard 'yasm''s exit code. To prevent this, use Bash's
    // 'pipefail' setting.
    let childProcess = child_process.exec(
      "set -o pipefail; yasm -p gas -L nasm -l /dev/stdout -o /dev/null - | cat",
      {
        shell: "/bin/bash",
        windowsHide: true,
      },
      (err, stdout, stderr) => {
        if (err !== null) {
          reject(err);
          return;
        }
        resolve(stdout);
      }
    );
    childProcess.stdin.write(code);
    childProcess.stdin.end();
  });
}

function hexStringToByteArray(hexes) {
  let outBytes = [];
  for (let i = 0; i < hexes.length; i += 2) {
    outBytes.push(parseInt(hexes.substr(i, 2), 16));
  }
  return outBytes;
}

async function mainAsync() {
  let files = process.argv.slice(2);
  if (files.length === 0) {
    console.error(`usage: ${process.argv[0]} ${process.argv[1]} file.cpp [file.cpp ...]`);
    process.exit(2);
  }
  for (let file of files) {
    let source = fs.readFileSync(file, 'utf-8');
    let newSource = await updateTestASMAsync(source);
    if (source !== newSource) {
      fs.writeFileSync(file, newSource, 'utf-8');
    }
  }
}

if (isModuleNodeMain(import.meta)) {
  await mainAsync();
}

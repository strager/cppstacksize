import assert from "node:assert";
import test from "node:test";
import { add } from "../src/add.mjs";

test("2+2=fish", (t) => {
  assert.strictEqual(add(2, 2), 4);
});

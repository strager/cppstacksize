import assert from "node:assert";

// Like assert.rejects, but returns the thrown error.
export async function assertRejectsAsync(callback, ...args) {
  let error;
  await assert.rejects(async () => {
    try {
      return await callback();
    } catch (e) {
      error = e;
      throw e;
    }
  }, ...args);
  return error;
}

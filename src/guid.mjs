export class GUID {
  #dataView;

  constructor(bytes) {
    console.assert(bytes.byteLength === 16);
    console.assert(bytes instanceof Uint8Array);
    this.#dataView = new DataView(bytes.slice().buffer);
  }

  toString() {
    return `${this.#dataView
      .getUint32(0, true)
      .toString(16)
      .padStart(8, "0")}-${this.#dataView
      .getUint16(4, true)
      .toString(16)
      .padStart(4, "0")}-${this.#dataView
      .getUint16(6, true)
      .toString(16)
      .padStart(4, "0")}-${this.#dataView
      .getUint16(8, false)
      .toString(16)
      .padStart(4, "0")}-${this.#dataView
      .getUint16(10, false)
      .toString(16)
      .padStart(4, "0")}${this.#dataView
      .getUint16(12, false)
      .toString(16)
      .padStart(4, "0")}${this.#dataView
      .getUint16(14, false)
      .toString(16)
      .padStart(4, "0")}`;
  }
}

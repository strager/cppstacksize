export function alignUp(n, alignment) {
  let mask = alignment - 1;
  return (n + mask) & ~mask;
}

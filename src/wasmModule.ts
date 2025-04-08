let wasm: any;

export async function loadWasmModule() {
  const response = await fetch('/emulator.wasm');
  const buffer = await response.arrayBuffer();
  const { instance } = await WebAssembly.instantiate(buffer, {
    env: {
      // import environment for wasm module (e.g., memory, print functions)
    }
  });
  wasm = instance.exports;
}

export function simulate(asmText: string): { json: string, text: string } {
  const encoder = new TextEncoder();
  const buffer = encoder.encode(asmText);
  const ptr = wasm.alloc(buffer.length);
  new Uint8Array(wasm.memory.buffer, ptr, buffer.length).set(buffer);
  wasm.simulate(ptr, buffer.length);

  const textPtr = wasm.getTextOutput();  // pointer to human-readable log
  const jsonPtr = wasm.getJsonOutput();  // pointer to JSON string
  const decoder = new TextDecoder();

  const textLog = decoder.decode(new Uint8Array(wasm.memory.buffer, textPtr, 1024)); // Adjust size
  const jsonStr = decoder.decode(new Uint8Array(wasm.memory.buffer, jsonPtr, 2048)); // Adjust size
  return { json: jsonStr, text: textLog };
}

export function reset() {
  wasm.reset();
}

export function getState(): { json: string, text: string } {
  const textPtr = wasm.getTextState();
  const jsonPtr = wasm.getJsonState();
  const decoder = new TextDecoder();

  return {
    text: decoder.decode(new Uint8Array(wasm.memory.buffer, textPtr, 1024)),
    json: decoder.decode(new Uint8Array(wasm.memory.buffer, jsonPtr, 2048))
  };
}

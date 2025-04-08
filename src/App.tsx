import { createSignal } from "solid-js";
import { loadWasmModule, simulate, reset, getState } from "./wasmModule";

function App() {
  const [asmInput, setAsmInput] = createSignal("");
  const [outputText, setOutputText] = createSignal("");
  const [outputJson, setOutputJson] = createSignal("");
  const [cpuState, setCpuState] = createSignal("");

  loadWasmModule();

  const runSimulation = () => {
    const result = simulate(asmInput());
    setOutputText(result.text);
    setOutputJson(result.json);
  };

  const resetCPU = () => {
    reset();
    setOutputText("");
    setOutputJson("");
    setCpuState("");
  };

  const fetchState = () => {
    const result = getState();
    setCpuState(result.text + "\n" + result.json);
  };

  return (
    <div style="font-family: monospace; padding: 1rem;">
      <h2>Intel 8086 Emulator Interface</h2>
      <textarea rows="10" cols="60" onInput={(e) => setAsmInput(e.currentTarget.value)} placeholder="Enter your assembly code here..."></textarea>
      <br />
      <button onClick={runSimulation}>Simulate</button>
      <button onClick={resetCPU}>Reset</button>
      <button onClick={fetchState}>State</button>

      <h3>Readable Output</h3>
      <pre>{outputText()}</pre>

      <h3>JSON Output</h3>
      <pre>{outputJson()}</pre>

      <h3>CPU Final State</h3>
      <pre>{cpuState()}</pre>
    </div>
  );
}

export default App;

const state = {
  module: null,
  engineReady: false,
  defaultTopologyText: "",
  defaultRovText: "",
  lastRibsCsv: "",
};

const elements = {
  form: document.querySelector("#sim-form"),
  targetAsn: document.querySelector("#target-asn"),
  announcementsFile: document.querySelector("#announcements-file"),
  topologyFile: document.querySelector("#topology-file"),
  rovFile: document.querySelector("#rov-file"),
  useDefaultTopology: document.querySelector("#use-default-topology"),
  useDefaultRov: document.querySelector("#use-default-rov"),
  runButton: document.querySelector("#run-button"),
  loadSampleButton: document.querySelector("#load-sample-button"),
  engineStatus: document.querySelector("#engine-status"),
  statusMessage: document.querySelector("#status-message"),
  summaryTarget: document.querySelector("#summary-target"),
  summaryRoutes: document.querySelector("#summary-routes"),
  summaryAses: document.querySelector("#summary-ases"),
  warnings: document.querySelector("#warnings"),
  resultsCaption: document.querySelector("#results-caption"),
  resultsBody: document.querySelector("#results-body"),
  downloadResults: document.querySelector("#download-results"),
};

function setStatus(label, message) {
  elements.engineStatus.textContent = label;
  elements.statusMessage.textContent = message;
}

function setEngineReady(ready) {
  state.engineReady = ready;
  elements.runButton.disabled = !ready;
}

async function fetchText(url) {
  const response = await fetch(url);

  if (!response.ok) {
    throw new Error(`Failed to load ${url}`);
  }

  return response.text();
}

async function readFileText(file) {
  if (!file) {
    return "";
  }

  return file.text();
}

function formatPath(path) {
  return `(${path.join(", ")}${path.length === 1 ? "," : ""})`;
}

function renderWarnings(warnings) {
  if (!warnings.length) {
    elements.warnings.hidden = true;
    elements.warnings.innerHTML = "";
    return;
  }

  elements.warnings.hidden = false;
  elements.warnings.innerHTML = warnings
    .map((warning) => `<div class="notice">${warning}</div>`)
    .join("");
}

function renderEmpty(message) {
  elements.resultsBody.innerHTML = `
    <tr class="empty-row">
      <td colspan="5">${message}</td>
    </tr>
  `;
}

function renderResults(result, targetAsn) {
  elements.summaryTarget.textContent = targetAsn;
  elements.summaryRoutes.textContent = String(result.target_routes.length);
  elements.summaryAses.textContent = String(result.total_ases);
  elements.resultsCaption.textContent = `Found ${result.target_routes.length} route${result.target_routes.length === 1 ? "" : "s"} at AS${targetAsn}.`;
  renderWarnings(result.warnings || []);

  if (!result.target_routes.length) {
    renderEmpty("No announcements reached the selected ASN for this input set.");
    return;
  }

  elements.resultsBody.innerHTML = result.target_routes
    .map((route) => `
      <tr>
        <td><strong>${route.prefix}</strong></td>
        <td><span class="path-chip">${formatPath(route.as_path)}</span></td>
        <td>${route.received_from}</td>
        <td>AS${route.next_hop_asn}</td>
        <td><span class="flag-chip ${route.rov_invalid ? "invalid" : ""}">${route.rov_invalid ? "Yes" : "No"}</span></td>
      </tr>
    `)
    .join("");
}

async function loadWasmModule() {
  const importedModule = await import("./assets/wasm/bgp_simulator.js");
  const factory = importedModule.default || window.createSimulatorModule;

  if (!factory) {
    throw new Error("The generated WASM loader was not found in web/assets/wasm.");
  }

  state.module = await factory();
}

function runSimulation(caidaData, rovCsv, announcementsCsv, targetAsn) {
  const caidaPtr = allocateString(caidaData);
  const rovPtr = allocateString(rovCsv);
  const announcementsPtr = allocateString(announcementsCsv);

  try {
    const resultPtr = state.module._run_simulation_json(
      caidaPtr,
      rovPtr,
      announcementsPtr,
      targetAsn
    );

    const resultJson = state.module.UTF8ToString(resultPtr);
    return JSON.parse(resultJson);
  } finally {
    state.module._free(caidaPtr);
    state.module._free(rovPtr);
    state.module._free(announcementsPtr);
  }
}

function allocateString(value) {
  const byteLength = state.module.lengthBytesUTF8(value) + 1;
  const ptr = state.module._malloc(byteLength);
  state.module.stringToUTF8(value, ptr, byteLength);
  return ptr;
}

async function prepareEngine() {
  try {
    setEngineReady(false);
    setStatus("Loading bundled data…", "Fetching the default topology and ROV dataset.");

    const [defaultTopologyText, defaultRovText] = await Promise.all([
      fetchText("./data/caida_topology.txt"),
      fetchText("./data/rov_asns.csv"),
    ]);

    state.defaultTopologyText = defaultTopologyText;
    state.defaultRovText = defaultRovText;

    setStatus("Loading WASM engine…", "Initializing the WebAssembly simulator in your browser.");
    await loadWasmModule();

    setStatus("Engine ready", "Upload announcements and run the simulation.");
    setEngineReady(true);
  } catch (error) {
    console.error(error);
    setStatus("Engine unavailable", error.message);
    renderEmpty("The WASM build is missing or failed to load. Build the browser bundle with the provided script in web/build-wasm.sh.");
  }
}

function syncOptionalInputs() {
  elements.topologyFile.disabled = elements.useDefaultTopology.checked;
  elements.rovFile.disabled = elements.useDefaultRov.checked;
}

async function handleSubmit(event) {
  event.preventDefault();

  if (!state.engineReady) {
    return;
  }

  const targetAsn = Number(elements.targetAsn.value);

  if (!Number.isInteger(targetAsn) || targetAsn <= 0) {
    setStatus("Invalid ASN", "Please enter a positive integer target ASN.");
    return;
  }

  const announcementsFile = elements.announcementsFile.files[0];

  if (!announcementsFile) {
    setStatus("Missing announcements CSV", "Upload the announcements file before running the simulator.");
    return;
  }

  if (!elements.useDefaultTopology.checked && !elements.topologyFile.files[0]) {
    setStatus("Missing topology file", "Upload a topology file or re-enable the bundled topology dataset.");
    return;
  }

  if (!elements.useDefaultRov.checked && !elements.rovFile.files[0]) {
    setStatus("Missing ROV CSV", "Upload an ROV CSV or re-enable the bundled ROV dataset.");
    return;
  }

  try {
    setStatus("Running simulation…", "Parsing inputs and propagating announcements client-side.");
    setEngineReady(false);

    const [announcementsCsv, customTopology, customRov] = await Promise.all([
      readFileText(announcementsFile),
      elements.useDefaultTopology.checked ? Promise.resolve(state.defaultTopologyText) : readFileText(elements.topologyFile.files[0]),
      elements.useDefaultRov.checked ? Promise.resolve(state.defaultRovText) : readFileText(elements.rovFile.files[0]),
    ]);

    const result = runSimulation(customTopology, customRov, announcementsCsv, targetAsn);

    if (!result.ok) {
      throw new Error(result.error || "Simulation failed.");
    }

    state.lastRibsCsv = result.ribs_csv;
    elements.downloadResults.disabled = false;
    renderResults(result, targetAsn);
    setStatus("Simulation complete", `Rendered ${result.target_routes.length} route${result.target_routes.length === 1 ? "" : "s"} for AS${targetAsn}.`);
  } catch (error) {
    console.error(error);
    renderEmpty("The simulation failed. Check the input files and try again.");
    elements.downloadResults.disabled = true;
    setStatus("Simulation failed", error.message);
  } finally {
    setEngineReady(true);
  }
}

async function loadSampleData() {
  try {
    const sample = await fetch("./data/sample_announcements.csv").then((response) => response.text());
    const file = new File([sample], "sample_announcements.csv", { type: "text/csv" });
    const transfer = new DataTransfer();
    transfer.items.add(file);
    elements.announcementsFile.files = transfer.files;
    elements.targetAsn.value = "174";
    setStatus("Sample loaded", "Sample announcements and target ASN 174 are ready to run.");
  } catch (error) {
    console.error(error);
    setStatus("Sample unavailable", "Could not load the bundled sample CSV.");
  }
}

function downloadResults() {
  if (!state.lastRibsCsv) {
    return;
  }

  const blob = new Blob([state.lastRibsCsv], { type: "text/csv;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = "ribs.csv";
  link.click();
  URL.revokeObjectURL(url);
}

elements.useDefaultTopology.addEventListener("change", syncOptionalInputs);
elements.useDefaultRov.addEventListener("change", syncOptionalInputs);
elements.form.addEventListener("submit", handleSubmit);
elements.loadSampleButton.addEventListener("click", loadSampleData);
elements.downloadResults.addEventListener("click", downloadResults);

syncOptionalInputs();
prepareEngine();

# Fourzo BGP Simulator Web App

This folder contains a static website for Cloudflare Pages. The site loads the simulator as WebAssembly and runs the full experiment in the browser.

## What it does

- loads a bundled CAIDA topology and ROV dataset by default
- accepts an announcements CSV upload from the user
- accepts a target ASN
- returns the announcements and AS paths visible at that target AS
- lets the user download the full generated `ribs.csv`

## Files

- `index.html`, `styles.css`, `app.js`: static frontend
- `data/`: bundled topology, ROV dataset, and sample announcement CSV
- `assets/wasm/`: generated Emscripten output goes here
- `_headers`: Cloudflare Pages cache headers
- `build-wasm.sh`: builds `assets/wasm/bgp_simulator.js` and `.wasm`

## Build the browser bundle

Install Emscripten, activate it, then run:

```bash
./web/build-wasm.sh
```

That generates:

- `web/assets/wasm/bgp_simulator.js`
- `web/assets/wasm/bgp_simulator.wasm`

## Local preview

From the project root, serve the `web` folder with any static file server after building the WASM bundle. For example:

```bash
python3 -m http.server 8000 --directory web
```

Then open:

```text
http://localhost:8000
```

## Cloudflare Pages

Use `web` as the build output directory. Since the site is static, Cloudflare Pages only needs to publish the folder after the WASM assets are committed.

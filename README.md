# LLPS_ACTIVE-BATH (C++)

Config-driven C++ analysis pipeline for LLPS droplet datasets collected under enzyme-driven “active bath” conditions.

This public version supports:
- **PC mode**: partition coefficient (PC) / droplet-stat processing across conditions and dates
- **PB mode**: photobleaching (PB)-related processing and normalization outputs

> **Not included:** an experimental KS-test module (removed from this public release, was using Mathematica/Matlab instead).

---

## Quickstart

```bash
cp LLPS_analysis_input_template.txt LLPS_analysis_input.txt
cmake --preset ucrt64-release
cmake --build --preset ucrt64-release-build
cd build-release
./LLPS_analysis
```
On Windows the executable may be `LLPS_analysis.exe`.

---

## Repository layout

This repo is meant to live inside a larger project folder layout, but the repo itself contains only code/config templates:

```text
LLPS_ACTIVE-BATH/          <-- (this repo; corresponds to "Codes/" in the thesis tree)
  CMakeLists.txt
  CMakePresets.json
  LLPS_analysis_input_template.txt
  include/
  src/
  build-debug/             <-- generated (do not commit)
  build-release/           <-- generated (do not commit)
```

## Expected data directory layout
The code assumes a Root / Parent / Child structure, where Parent and Child correspond to the `parent:` and `child:` entries in the config file.
```text
Root/
  Parent/
    Child/
      file_info/                     <-- required (name must match)
        <date>_Stat-info.csv         <-- required for PC mode
      raw/                           <-- required (name must match)
        <date>/                      <-- must match `date_file` in config
          Z-<file>_<frame>_Droplet_stats.csv   <-- droplet z-scan data CSV (example)
          <file>-<n>.csv                       <-- PB data CSV (example)
          <file>-<n>_R.csv                     <-- PB reference data CSV (example)
      normalized/                    <-- used by PB mode (name must match)
        <date>/                      <-- must match `date_file` in config
  Codes/                             <-- this repo location (suggested)
```
Directory names must match exactly as shown above (e.g., `raw/`, `file_info/`, `normalized/`). If your layout differs, adjust the path parsing logic in the code.
The code assumes PC and PB raw data are under different `Child/` directories.

## Configuration
The program requires a config file. Start from the template:
- Copy `LLPS_analysis_input_template.txt` → `LLPS_analysis_input.txt`
- Place `LLPS_analysis_input.txt` in the repo root (same level as `src/` and `include/`)
### Important: where the program looks for the config
The current `main.cpp` expects `LLPS_analysis_input.txt` to be located in the parent directory of the working directory (`Codes/` directory shown in the directory layout).
That means the intended workflow is:
- Keep `LLPS_analysis_input.txt` in the repo root
- Run the executable from a build directory (e.g., `build-debug/` or `build-release/`)

## Build
### Using presets (recommended)
```bash
cmake --preset ucrt64-debug
cmake --build --preset ucrt64-debug-build
```
Release:
```bash
cmake --preset ucrt64-release
cmake --build --preset ucrt64-release-build
```
### Generic CMake
```bash
cmake -S . -B build
cmake --build build -j
```
## Run
Example (from `build-debug/` so the parent directory (`Codes/`) contains `LLPS_analysis_input.txt`):
```bash
cd build-debug
./LLPS_analysis
```
On Windows the executable may be `LLPS_analysis.exe`.

## Input file: `<date>_Stat-info.csv` (PC mode)
For partition coefficient / droplet-stat analysis, a CSV file is required in:
```text
<Parent>/<Child>/file_info/
```
Filename must match the date listed in your config:
```text
<date>_Stat-info.csv
```
### Required headers (do not change)
The following headers (for `<date>_Stat-info.csv`) are hard-coded in the parser, so do not rename them:
```text
File#    Frame    Time start    Time File    Del_T(min)    Del_T_avg(min)    Exclude    Exc-element    Note
```
### Meaning/rules
- `File#`, `Frame`, and `Del_T(min)` must be numeric-like entries (strings that parse as numbers are OK).
- To skip an entire frame: set `Exclude` to `Y` and leave `Exc-element` empty.
- To skip specific droplets: set `Exclude` to `Y` and put droplet indices in `Exc-element`.
- `Note` is ignored by the code (free text).
- Avoid empty lines in the CSV (empty lines may trigger exceptions).

### Example rows
```text
File#  Frame  Time start  Time File  Del_T(min)  Del_T_avg(min)  Exclude  Exc-element  Note
011    035    6:54 AM     9:26 AM    152                        Normal run, scan all droplets
011    035    6:54 AM     9:26 AM    152                         Y                   Skips entire frame
011    035    6:54 AM     9:26 AM    152                         Y       6_7_13      Only skips droplet 6, 7, and 13
```
### Where droplet-stat CSVs go (PC mode)
All droplet-stat CSVs for a given date must be stored under:
```text
<Parent>/<Child>/raw/<date>/
```
The code expects per-frame CSV outputs from FIJI/ImageJ droplet detection (naming conventions are configurable only by editing code).

### Outputs
After PC analysis, the code generates the following directories under:
```text
<Parent>/<Child>/raw/<date>/
```
- `Max/` Records the maximum droplet intensity for each droplet per file number.
- `ABN_Append/` Records droplets detected after half of the total number of frames has already been read (recommended to visually verify whether they should be rejected).
- `Exclude_Append/` Records droplets outside preset bounds (bounds are defined in the code; see PC algorithm implementation).

## Input files: `<file>-<n>.csv` and `<file>-<n>_R.csv` (PB mode)
This part of the code takes a photobleached droplet intensity data (over time) and a reference droplet intensity data (over time) for normalization. For the normalization method, see https://arxiv.org/abs/2510.26659.
### Meaning/rules
- `<n>` is the index of the photobleached droplet. The `_R` suffix indicates the corresponding **reference** droplet.
- Every photobleached droplet file must have a **corresponding reference** file with the same index (e.g., `<file>-3.csv` pairs with `<file>-3_R.csv`).
- All PB input files must be placed under `raw/<date>/`.
- Ensure `PB_correction:` is set to `true` in `LLPS_analysis_input.txt` when running PB mode.

### Outputs
After PB normalization, the code generates normalized PB data with the filename `<file>-<n>_norm.csv` under `normalized/<date>/`.

## Known limitations
- Research-driven directory and filename conventions (tailored to my dataset structure).
- Parameter validation and error messages can be improved.

## Citation
If you use this code in academic work, please cite:
- Enzyme Active Bath Affects Protein Condensation (https://arxiv.org/abs/2510.26659)
- [This repository](https://github.com/kchingphy/LLPS_ACTIVE-BATH)


## License
MIT (see LICENSE).

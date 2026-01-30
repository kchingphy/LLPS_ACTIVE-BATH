# LLPS_ACTIVE-BATH (C++)

Config-driven C++ analysis pipeline for LLPS droplet datasets collected under enzyme-driven “active bath” conditions.

This public version supports:
- **PC mode**: partition coefficient (PC) / droplet-stat processing across conditions and dates
- **PB mode**: photobleaching (PB)-related processing and normalization outputs

> **Not included:** an experimental KS-test module (removed from this public release, was using Mathematica/Matlab instead).

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
The code assumes a Root / Parent / Child structure, where Parent and Child correspond to the parent: and child: entries in the config file.
```text
Root/
  Parent/
    Child/
      file_info/           <-- required (name must match)
        <date>_Stat-info.csv
      raw/                 <-- required (name must match)
        <date>/            <-- date folder (must match date_file entry)
          Z-<file>_<frame>_Droplet_stats.csv   (droplet z-scan data file with example naming)
          <file>-<number>.csv   (droplet PB data file with example naming)
          <file>-<number>_R.csv   (droplet PB reference data file with example naming)  
  Codes/                   <-- this repo lives here (suggested)
```
Directory with name must match reqires the directory to be name exactly as shown above, else you will need to adjust the parsing logic in the code.
The code assumes PC and PB raw data are under different `Child/` directory. 

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
Using presets (recommended)
```text
cmake --preset ucrt64-debug
cmake --build --preset ucrt64-debug-build
```
Release:
```text
cmake --preset ucrt64-release
cmake --build --preset ucrt64-release-build
```
Generic CMake
```text
cmake -S . -B build
cmake --build build -j
```
## Run
Example (from `build-debug/` so the parent directory (`Codes/`) contains `LLPS_analysis_input.txt`):
```text
cd build-debug
./LLPS_analysis
```

# Input file: `<date>_Stat-info.csv`
For partition coefficient / droplet-stat analysis, a CSV file is required in:
```text
<Parent>/<Child>/file_info/
```
Filename must match the date listed in your config:
```text
<date>_Stat-info.csv
```
## Required headers (do not change)
The following headers (for `<date>_Stat-info.csv`) are hard-coded in the parser, so do not rename them:
```text
File#    Frame    Time start    Time File    Del_T(min)    Del_T_avg(min)    Exclude    Exc-element    Note
```
## Meaning/rules
- `File#`, `Frame`, and `Del_T(min)` must be numeric-like entries (strings that parse as numbers are OK).
- To skip an entire frame: set `Exclude` to `Y` and leave Exc-element empty.
- To skip specific droplets: set Exclude to `Y` and put droplet indices in `Exc-element`.
- `Note` is ignored by the code (free text).
- Avoid empty lines in the CSV (empty lines may trigger exceptions).

## Example rows
```text
File#  Frame  Time start  Time File  Del_T(min)  Del_T_avg(min)  Exclude  Exc-element  Note
011    035    6:54 AM     9:26 AM    152                        Normal run, scan all droplets
011    035    6:54 AM     9:26 AM    152                         Y                   Skips entire frame
011    035    6:54 AM     9:26 AM    152                         Y       6_7_13      Only skips droplet 6, 7, and 13
```
## Where droplet-stat CSVs go (PC mode)
All droplet-stat CSVs for a given date must be stored under:
```text
<Parent>/<Child>/raw/<date>/
```
The code expects per-frame CSV outputs from FIJI/ImageJ droplet detection (naming conventions are configurable only by editing code).

## Outputs
After PC analysis, the code generates the following directories under:
```text
<Parent>/<Child>/raw/<date>/
```
- `Max/` Records the maximum droplet intensity for each droplet per file number.
- `ABN_Append/` Records droplets detected after half of the total number of frames has already been read (recommended to visually verify whether they should be rejected).
- `Exclude_Append/` Records droplets outside preset bounds (bounds are defined in the code; see PC algorithm implementation).

## Known limitations
- Research-driven directory and filename conventions (tailored to my dataset structure).
- Parameter validation and error messages can be improved.

# Citation
If you use this code in academic work, please cite:
- Enzyme Active Bath Affects Protein Condensation (DOI: https://doi.org/10.48550/arXiv.2510.26659)
- This repository


# License
MIT (see LICENSE).

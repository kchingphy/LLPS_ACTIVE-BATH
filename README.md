# LLPS_ACTIVE-BATH (Fiji/ImageJ + C++)

Reproducible microscopy image-analysis workflow for LLPS droplet datasets collected under enzyme-driven “active bath” conditions. The repository combines a Fiji/ImageJ droplet-detection macro with a config-driven C++ analysis pipeline.

This public version supports:
- **Fiji/ImageJ preprocessing**: droplet detection, background measurement, per-droplet intensity/shape measurements, visual-QC image outputs, and CSV export
- **PC mode**: partition coefficient (PC) / droplet-stat processing across conditions and dates
- **PB mode**: photobleaching (PB)-related processing and normalization outputs

> **Not included:** an experimental KS-test module (removed from this public release; statistical comparisons were handled separately in Mathematica/MATLAB).

---

## Workflow overview

```text
Raw microscopy images
  -> Fiji/ImageJ macro
       - threshold-based droplet detection
       - background and droplet measurements
       - per-frame *_Droplet_stats.csv outputs
  -> C++ pipeline
       - PC mode: partition-coefficient / droplet-stat processing
       - PB mode: photobleaching normalization
  -> QC and normalized analysis outputs
```

## Quickstart

### C++ pipeline

```bash
cp LLPS_analysis_input_template.txt LLPS_analysis_input.txt
cmake --preset ucrt64-release
cmake --build --preset ucrt64-release-build
cd build-release
./LLPS_analysis
```
On Windows the executable may be `LLPS_analysis.exe`.

### Fiji/ImageJ macro

Open `fiji/Partition_Coeff_Macro_stack_template.ijm` in Fiji/ImageJ, edit the directory variables near the top of the file, and run the macro on a folder of TIFF images named with the pattern:

```text
Z-<file>_<frame>.tif
```

The macro writes per-frame droplet-stat CSV files named:

```text
Z-<file>_<frame>_Droplet_stats.csv
```

These CSV files are the droplet-detection outputs used as inputs for the downstream PC analysis.

---

## Repository layout

This repo is meant to live inside a larger project folder layout, but the repo itself contains only code/config templates:

```text
LLPS_ACTIVE-BATH/          <-- (this repo; corresponds to "Codes/" in the thesis tree)
  CMakeLists.txt
  CMakePresets.json
  LLPS_analysis_input_template.txt
  fiji/
    Partition_Coeff_Macro_stack_template.ijm
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


## Fiji/ImageJ droplet-detection macro

The Fiji/ImageJ macro in `fiji/Partition_Coeff_Macro_stack_template.ijm` generates the per-frame droplet-stat CSV files used by PC mode.

### Attribution

The macro was adapted from Mahdi Moosa's `PartitionCoefficient-ImageJ_Macro` GitHub repository:

```text
https://github.com/Mahdi-Moosa/PartitionCoefficient-ImageJ_Macro
```

This version was modified for this LLPS workflow, including z-stack/image-sequence file naming, local directory conventions, background/droplet measurements, visual-QC image outputs, and CSV outputs for downstream C++ analysis.

### Macro input layout

The template macro uses the following local image-processing layout:

```text
rootDir/
  dateDir/
    conditionDir/
      fileSetDir/
        raw_data_z/      input TIFF images, e.g. Z-011_035.tif
        analyzed/        optional analyzed/segmentation TIFFs
      results/           output droplet-stat CSV files
```

Edit these variables near the top of the macro before running:

```text
rootDir
dateDir
conditionDir
fileSetDir
```

### Macro behavior

For each TIFF file whose name starts with `Z-`, the macro:

- resets the image scale to pixel units
- estimates the background using a duplicate image, median filtering, and `MinError dark` thresholding
- segments droplets using median filtering, `Intermodes dark` thresholding, and watershed separation
- measures droplet area, intensity statistics, centroid/geometry values, circularity, and related features
- saves an analyzed TIFF image for visual QC when `saveAnalyzedImages = true`
- saves a CSV file named `<input_base>_Droplet_stats.csv`

The macro assumes bright droplets on a darker background. If automatic thresholding fails or gives poor segmentation, inspect the image focus, contrast, and foreground/background separation.

### Connecting Fiji/ImageJ outputs to the C++ PC mode

The C++ PC mode expects droplet-stat CSV files under:

```text
<Parent>/<Child>/raw/<date>/
```

The macro template writes CSV files to its `results/` directory. After running the macro, either:

- copy the `*_Droplet_stats.csv` files into the C++ PC input directory, or
- edit the macro `outputDir` so it writes directly to the C++ PC input directory.

Keep the filename pattern unchanged unless you also update the C++ filename parsing logic.


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
011    035    6:54 AM     9:26 AM    152                                               Normal run, scan all droplets
011    035    6:54 AM     9:26 AM    152                         Y                     Skips entire frame
011    035    6:54 AM     9:26 AM    152                         Y        6_7_13       Only skips droplet 6, 7, and 13
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
- Fiji/ImageJ thresholding can fail or give poor segmentation when droplets are out of focus or foreground/background contrast is low.
- The Fiji/ImageJ macro and C++ pipeline currently use separate directory conventions; outputs may need to be copied or the macro `outputDir` may need to be edited.
- Parameter validation and error messages can be improved.

## Citation and reuse
If you use this code in academic work, please cite:
- Enzyme Active Bath Affects Protein Condensation (https://arxiv.org/abs/2510.26659)
- [This repository](https://github.com/kchingphy/LLPS_ACTIVE-BATH)

The Fiji/ImageJ macro was adapted from Mahdi Moosa's `PartitionCoefficient-ImageJ_Macro` repository. Please preserve that attribution if you reuse or modify the macro.


## License
MIT (see LICENSE).

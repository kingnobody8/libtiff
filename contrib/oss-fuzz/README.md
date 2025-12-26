# LibTIFF OSS-Fuzz Integration

This directory contains comprehensive fuzzing support for libtiff, designed for
integration with [Google's OSS-Fuzz](https://github.com/google/oss-fuzz) continuous
fuzzing infrastructure.

## Fuzzers

| Fuzzer | Description | Target Functions |
|--------|-------------|------------------|
| `tiff_read_rgba_fuzzer` | RGBA image reading | `TIFFReadRGBAImage`, `TIFFReadScanline` |
| `tiff_read_strip_fuzzer` | Strip/tile reading | `TIFFReadEncodedStrip`, `TIFFReadRawStrip`, `TIFFReadEncodedTile` |
| `tiff_read_dir_fuzzer` | Directory parsing | `TIFFReadDirectory`, `TIFFReadEXIFDirectory`, `TIFFGetField` |
| `tiff_print_dir_fuzzer` | Print/dump functions | `TIFFPrintDirectory`, size calculation functions |
| `tiff_color_fuzzer` | Color conversion | `TIFFYCbCrToRGBInit`, `TIFFCIELabToRGBInit`, `TIFFReadRGBAStrip` |
| `tiff_write_read_fuzzer` | Write round-trip | `TIFFWriteScanline`, `TIFFWriteEncodedTile`, `TIFFClientOpen` |
| `tiff_codec_fuzzer` | Codec implementations | LZW, PackBits, FAX3/4, JPEG, Deflate decompression |
| `tiff_bigtiff_fuzzer` | BigTIFF support | 64-bit offset functions, `TIFFIsBigTIFF` |
| `tiff_multipage_fuzzer` | Multi-page handling | `TIFFSetDirectory`, `TIFFSetSubDirectory`, directory navigation |
| `tiff_custom_tag_fuzzer` | Custom tag handling | `TIFFFieldWithTag`, `TIFFGetTagListEntry`, field enumeration |
| `tiff_ojpeg_fuzzer` | Old-style JPEG | OJPEG reconstruction, legacy JPEG parsing |
| `tiff_predictor_fuzzer` | Predictor processing | Horizontal, floating-point predictors with LZW/Deflate |
| `tiff_open_options_fuzzer` | Open options API | `TIFFOpenOptionsSet*`, memory limits, error handlers |

## Files

```
contrib/oss-fuzz/
├── build.sh                    # OSS-Fuzz build script
├── CMakeLists.txt              # CMake build support for fuzzers
├── README.md                   # This file
├── tiff.dict                   # Fuzzing dictionary
├── corpus/                     # Seed corpus samples
│   ├── *.tiff                  # Test images
│   └── minimal_*.tiff          # Minimal valid samples
├── *_fuzzer.cc                 # Fuzzer source files
└── *_fuzzer.options            # Fuzzer-specific options
```

## Building Fuzzers

### With OSS-Fuzz

```bash
cd oss-fuzz
python3 infra/helper.py build_image libtiff
python3 infra/helper.py build_fuzzers --sanitizer address libtiff
python3 infra/helper.py check_build libtiff
```

### With CMake (standalone testing)

```bash
mkdir build && cd build
cmake .. -DBUILD_FUZZERS=ON -DFUZZER_STANDALONE=ON
make
```

### Manual compilation

```bash
g++ -std=c++11 -I/path/to/libtiff -DSTANDALONE \
    tiff_read_rgba_fuzzer.cc -o fuzzer \
    -ltiffxx -ltiff -ljpeg -lz
./fuzzer corpus/sample.tiff
```

## Sanitizer Support

| Sanitizer | Status | Notes |
|-----------|--------|-------|
| AddressSanitizer (ASan) | ✅ Enabled | |
| UndefinedBehaviorSanitizer (UBSan) | ✅ Enabled | |
| MemorySanitizer (MSan) | ✅ Enabled | Requires building deps from source |

MSan support requires building all dependencies (zlib, libjpeg-turbo, jbigkit,
liblzma) from source with MSan instrumentation. Optional codecs (zstd, webp,
lerc, libdeflate) are disabled in MSan builds.

## Fuzzing Engines

- libFuzzer (default)
- AFL++
- honggfuzz

## Seed Corpus

The `corpus/` directory contains seed samples including:
- Real-world TIFF images from the test suite
- Minimal valid TIFF files for various configurations
- Edge cases (truncated, minimal headers, etc.)

## Dictionary

`tiff.dict` contains TIFF-specific tokens:
- Magic bytes (II, MM)
- Tag IDs
- Compression types
- Data types
- Photometric values

## CI Integration

See `.gitlab-ci-fuzzing.yml` for GitLab CI configuration that:
- Builds fuzzers with sanitizers
- Runs regression tests against corpus
- Supports extended fuzzing runs

## Contributing

When adding new fuzzers:
1. Create `new_fuzzer.cc` with `LLVMFuzzerTestOneInput` function
2. Add to the `FUZZERS` list in `build.sh`
3. Add to CMakeLists.txt
4. Create `.options` file if needed (timeouts, memory limits)
5. Add relevant samples to corpus

## Coverage

Current estimated coverage: ~75-85% (up from ~42% with single fuzzer)

The comprehensive fuzzer suite now covers:
- All major read/write paths
- All supported compression codecs
- Legacy formats (OJPEG, BigTIFF)
- Edge cases (predictors, color conversion, multi-page)
- Memory safety (TIFFOpenOptions API)

## References

- [OSS-Fuzz Documentation](https://google.github.io/oss-fuzz/)
- [libFuzzer Documentation](https://llvm.org/docs/LibFuzzer.html)
- [LibTIFF Documentation](https://libtiff.gitlab.io/libtiff/)

# Build and install

## GitHub Actions (no local SDK or administrator access)

The workflow `.github/workflows/build-vita.yml` first runs all four portable suites on Ubuntu, including real libarchive/Expat integration. It then builds the native release in the full official VitaSDK image `vitasdk/vitasdk:2026.08-20260815`. The dated image tag fixes the SDK image rather than following `latest`.

1. Create a GitHub repository (private is fine), and push this project to its `main` branch. `.tools/` is excluded; do not upload downloaded SDKs or PowerShell.
2. Open **Actions → Build Vreed**. A push to `main` starts it automatically; **Run workflow** starts it manually once the workflow is on the default branch.
3. After both jobs succeed, open the run and download **Artifacts → Vreed-VPK**. Extract the downloaded ZIP to get `Vreed.vpk` and its SHA-256 checksum.
4. Copy the VPK to the Vita and install it with VitaShell.

No secrets are required. The workflow only has read access to repository contents and does not publish a GitHub Release. If a job fails, open its failing step to inspect the compiler output. The first remote run passed on 2026-10-06, including all four tests and native VPK generation. A successful build does not establish hardware performance.

References: [official VitaSDK container](https://github.com/vitasdk/docker), [GitHub artifact upload](https://github.com/actions/upload-artifact).

## Local Vita build

Use a Linux VitaSDK installation (or WSL2 with a Linux distribution). This project does not install SDKs or modify system configuration automatically.

1. Follow the official [VitaSDK setup](https://vitasdk.org/). Install its port libraries: vita2d, FreeType, libpng, libjpeg, libarchive, Expat, bzip2, zstd and zlib. Package names can differ from linker names. Check your SDK headers and static libraries before building.
2. Export `VITASDK` and add `$VITASDK/bin` to `PATH`.
3. Build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
```

Expected artifact: `build/Vreed.vpk`. Application identity is centralized in `config/identity.cmake`; storage paths and limits are in `src/app/config.hpp`.

Install the VPK with VitaShell on a homebrew-enabled Vita. Generate the original sample with `python tools/make_sample.py`, then copy `assets/VitaReader-test.epub` to `ux0:/data/VitaReader/books/` using VitaShell USB or FTP. Launch VitaReader and open it with Cross.

Custom icon and LiveArea assets use the user's supplied logo, stored in `assets/branding`. The original source is preserved alongside package-sized PNGs. Verify the bubble and LiveArea on hardware after updating.

## Host checks

The layout module has no platform dependencies:

```sh
c++ -std=c++17 -Wall -Wextra -Isrc tests/layout_test.cpp src/reader/layout.cpp src/reader/settings.cpp src/rendering/page_style.cpp -o layout_test
./layout_test
c++ -std=c++17 -Wall -Wextra -Isrc tests/session_test.cpp src/reader/session.cpp src/reader/layout.cpp -o session_test
./session_test
```

The complete Vita link and resulting package require VitaSDK. Host checks cannot establish compatibility or performance on hardware.

On a host with CMake, a C++ compiler, libarchive and Expat development packages, run the complete portable test suite in a separate build directory:

```sh
cmake -S . -B build-host -DVITAREADER_HOST_TESTS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The EPUB integration test checks metadata, spine order, emphasis preservation, bidirectional chapter navigation, stale resume clamping and preservation of the current session after a failed open.

If libarchive/Expat are unavailable, add `-DVITAREADER_EPUB_TESTS=OFF` to run the portable layout/theme and synthetic-session tests only. The real application retains the existing EPUB parser.

## MuPDF and corresponding sources

Before native CMake configuration run `sh tools/build_mupdf.sh vita` in the VitaSDK Linux environment. CI also builds the same pinned engine for portable PDF tests. MuPDF 1.28.5 and its exact submodule revisions are downloaded automatically.

Each successful Actions run provides **Vreed-VPK** and **Vreed-Sources** side by side. The source archive contains Vreed, its build recipes, MuPDF and its dependencies, excluding generated build outputs. Vreed is now distributed under AGPL-3.0-or-later with the user's agreement.

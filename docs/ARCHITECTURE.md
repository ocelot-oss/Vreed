# Architecture

`platform/` polls Vita controls into platform-independent actions. `app/` owns library/reader state transitions. `storage/` scans files and persists positions. `epub/` reads the ZIP package and parses XML into blocks and styled runs. `reader/` paginates and manages chapter transitions. `rendering/` measures and draws with the same vita2d font. `main.cpp` only starts the application and exits the process.

The session retains the current chapter and one adjacent cache for spread rendering. Chapter transitions stage the destination before committing, so parser failures preserve the current reading screen. ZIP entries are capped at 4 MiB of decompressed data; scans cap at 8192 entries. ZIP data is never extracted to the filesystem. XML entity declarations are rejected and external parameter entities disabled. Images become placeholders.

Supported subset: unencrypted EPUB packages, container rootfile, OPF metadata and linear XHTML spine, paragraphs, headings, basic line breaks and UTF-8 text. Bold and italic are retained in the intermediate representation, but currently drawn in the regular system font. CSS, malformed HTML, named HTML entities beyond XML's built-ins, remote resources, DRM and full Unicode shaping are unsupported. PGF coverage for French accents must be checked on hardware; supplementary-plane characters are not guaranteed by this upstream renderer.

Pagination uses actual font advance widths and theme-specific geometry from `reader/settings`. Baseline spacing uses measured heading height and the selected line gap. Oversized tokens split at UTF-8 boundaries. Pages record source offsets for reflow and are independent of background decoration. A spread uses the same paginator with a narrower viewport and an adjacent-page view. Styled typography will still require positioned runs instead of plain line strings.

The library remains dark. BOOK, CLEAN and OLED NIGHT share content and pagination but have distinct geometry, colors and decoration. A compact reading overlay adjusts mode and typography. The separate ribbon is a temporary bookmark component. See `docs/MILESTONE_2.md`. Cover paths remain recorded but undecoded. Filename fallback keeps scanning fast. Errors appear in a strip; no book failure terminates the application loop.

Persistence uses a stable FNV-1a path key plus the original path inside each record to detect collisions. A temporary file is flushed and renamed over the prior checkpoint. This is best-effort atomic replacement, not a power-loss guarantee; Vita filesystem replacement behavior must be verified. Failed saves are visible. Checkpoints on every turn prioritize resume reliability but may need batching after flash-write and latency measurements.

## Milestone 3 interaction architecture

`reader/transition` owns time and gesture state (IDLE, DRAGGING, COMPLETING, CANCELLING), easing and one-shot sound/commit events. `reader/turn_transaction` stages a copied session and commits it only when the transition completes. Chapter caches are shared; cancelling drops the staged session without changing the live text anchor. App interruptions finish accepted automatic turns and cancel unaccepted gestures before saving. A frame gap above 250 ms is a conservative interruption proxy, not a verified Vita focus callback.

`input/touch` tracks a single contact and cancels conflicting contacts. Vita front-panel coordinates are normalized into the 960×544 canvas. BOOK outer-edge drags drive normalized progress; release uses distance plus recent velocity. Physical repeats resolve the current transaction before requesting another.

`rendering/curl` computes fixed-size mirrored strip geometry. The native renderer caches two full scenes in reusable render targets and uses 16 strips, soft shadow bands and a shaded backside. Spread backside UVs select the destination opposite sheet. Text and paper are captured before animation, keeping parsing, pagination and font drawing outside each animated frame. This is a low-cost visual approximation, not a physical paper simulation. Allocation failure falls back to NONE. GPU vertices use vita2d's transient pool so their lifetime extends through submission.

`audio/events` supplies a bounded queue and original procedural paper samples. `platform/vita_audio` consumes events in a blocking audio worker; the UI never calls the blocking output API. Page sounds are issued only after acceptance, once per transaction. `storage/settings` persists validated preferences with temporary-file replacement. `RibbonMotion` animates the separate bookmark component without changing pagination.

Two 960×544 RGBA surfaces require approximately 4 MiB of color storage; their depth buffers may add another 4 MiB before alignment/driver overhead. During preparation, staged sessions can temporarily retain additional chapter caches. Frame statistics are diagnostic elapsed-time counters, not GPU profiling or proof of 60 FPS. See [milestone 3](MILESTONE_3.md).

## Additional upstream references

- [Vita touch API](https://github.com/vitasdk/vita-headers/blob/master/include/psp2/touch.h).
- [Vita audio output API](https://github.com/vitasdk/vita-headers/blob/master/include/psp2/audioout.h).
- [vita2d texture implementation](https://github.com/xerpi/libvita2d/blob/master/libvita2d/source/vita2d_texture.c): render targets and textured primitives.

## Other verified upstream references

- [vita2d public header](https://github.com/xerpi/libvita2d/blob/master/libvita2d/include/vita2d.h): PGF font drawing, measurement and drawing lifecycle.
- [vita2d PGF implementation](https://github.com/xerpi/libvita2d/blob/master/libvita2d/source/vita2d_pgf.c): UTF-8 decoding and system font loading.
- [Vita controller header](https://github.com/vitasdk/vita-headers/blob/master/include/psp2/ctrl.h): non-blocking positive controller polling.
- [VitaSDK libarchive recipe](https://github.com/vitasdk/packages/blob/master/libarchive/VITABUILD): Vita port and compression dependencies.
- [VitaShell CMake](https://github.com/TheOfficialFloW/VitaShell/blob/master/CMakeLists.txt): Expat/libarchive linkage and VPK macros in an existing application.

SDK versions are not pinned until a real toolchain build establishes a known-good baseline.

# Milestone 3 — page interaction

Source implementation completed on 2026-10-05. Native VitaSDK build, VPK installation and physical-device acceptance remain outstanding. The Windows preview generator was not changed or investigated for this milestone.

## Behavior and architecture

`PageTransition` is independent of Vita APIs. It owns IDLE, DRAGGING, COMPLETING and CANCELLING states, normalized progress, direction, timing, release velocity and one-shot events. `ReaderTurn` stages a destination session, sharing existing chapter caches, and commits its position only at completion. A failed destination parse or cancelled gesture preserves the live page. Checkpoints always use the committed session.

BOOK defaults to CURL at 360 ms and permits NONE or SLIDE. CLEAN and OLED NIGHT default to SLIDE at 160 ms and permit NONE. Automatic transitions use smoothstep easing; touch progress follows the finger directly, then eases toward completion or cancellation. Constants, including strip count and timing, are centralized in `reader/transition.hpp`.

Repeated physical input resolves the current turn before staging the next. Accepted automatic/completing turns commit; dragging/cancelling turns return to their source. There is no unbounded navigation queue. Chapter-boundary errors leave the old page readable. Book boundaries produce a brief edge highlight in BOOK. Opening settings, returning to the library and START settle the transaction before saving. A frame gap above 250 ms invokes the same policy as a conservative interruption safeguard; actual Vita suspend/focus handling is not validated.

## Curl technique and spread semantics

Two reusable 960×544 render targets capture the source and destination scenes before movement. The renderer draws the destination underneath 16 deformed strips from the source. Cosine projection rotates the sheet, a restrained sinusoidal bow adds curvature, the touch height affects its vertical bend, and eight translucent shadow bands follow the edge. The backside is shaded paper for a single page; a spread samples the opposite destination sheet on its backside. The stationary source sheet is retained through the first half of a spread turn.

Forward spread navigation stages two pages, including across chapters: the old right sheet turns left and exposes the next pair. Reverse navigation mirrors the geometry and sheet mapping. An unmatched final right page remains blank. Pagination geometry is unchanged during movement. This is a perceptual curl approximation; gutter handoff, paper illusion, backside and shadow tuning need native visual review.

Texture allocation is lazy and reused. Failure disables animation surfaces and falls back to an instant change with a visible message. Animation geometry uses fixed arrays and vita2d's transient GPU vertex pool; text is not redrawn or repaginated during the movement. Capture, chapter parsing and checkpoint writes can still cause latency around a turn and must be measured on hardware.

## Touch

Front-touch coordinates are normalized using the panel's reported display range. Only BOOK outer page-edge zones (48 px) initiate a turn; the spread gutter does not. The tracker owns one contact until release. Multiple contacts, an ID swap or invalid polling cancel a dragging gesture and block it until fingers lift.

A release commits at 35% travel, or with recent forward velocity above 0.9 page widths/s after at least 6% travel. Velocity older than 80 ms cannot trigger a flick. Reversing direction reduces progress; insufficient travel eases back without a position change or page sound. The touch's vertical origin subtly changes the bend. Hardware calibration and responsiveness remain unverified.

## Sound and ribbon

Paper sounds are original deterministic procedural development placeholders: short filtered-noise envelopes, generated once, not downloaded or copied from a recording. Turn samples are 140 ms at 48 kHz mono; bookmark samples are 50 ms. A bounded queue feeds a dedicated Vita worker with 512-sample output blocks. Blocking audio output never runs on the UI thread. Device/worker failure silently disables output.

Accepted turns emit one sound when completing progress crosses 52%; rapid settlement emits it once if necessary. Dragging and cancelled gestures emit none. Forward/backward and ribbon insert/remove have separate events. Sounds may replace the previous voice during rapid navigation. The toggle and four volume levels (mute, 25%, 50%, 75% of the quiet generated signal) persist with all reading preferences.

The separate ribbon component enters/exits over 220 ms and can reverse smoothly midway. It follows the bookmarked passage without changing pagination. Bookmark state itself is still temporary and is reset on book opening.

## Tests and reproduction

Portable tests used Zig 0.16 on Windows, with assertions enabled:

- `layout_test`: UTF-8, anchors, 162 settings combinations, gutters, large-font fallback and OLED black.
- `session_test`: synthetic cross-chapter spreads, final pages, reverse navigation, reflow and failure rollback.
- `interaction_test`: automatic completion, cancellation, fresh/stale flicks, touch ownership, mirrored finite curl geometry, ribbon reversal, settings overwrite/corruption, audio queue/mute and transactional repeated/spread navigation.
- `epub_test`: real libarchive/Expat parsing of `assets/VitaReader-test.epub`, metadata, styled semantics, accents, chapter navigation, stale resume and failed-open preservation.

The interaction and real EPUB binaries executed successfully from the current sources. Previously built layout and session binaries also executed successfully at the start of validation. Both suites then recompiled successfully against the current sources, but Windows application-control policy blocked execution of the newly built layout/session binaries. Their fresh execution remains outstanding; no policy bypass was attempted. This does not concern the untouched preview generator.

CMake's `VITAREADER_HOST_TESTS=ON` registers these tests with CTest. The EPUB suite requires host libarchive and Expat; use `VITAREADER_EPUB_TESTS=OFF` only when those are unavailable. In this Windows environment the real EPUB test used isolated MSYS2 libarchive 3.8.9 and Expat 2.8.5. `tools/fetch_host_test_deps.py` downloads hash-pinned packages to ignored `.tools/hostdeps`, extracts headers/import libraries/DLLs/licenses without running package scripts, and uses existing Git for Windows compression DLLs. It is a Windows test convenience, not a Vita build dependency.

## Performance assumptions and device gate

Two RGBA color surfaces total about 4 MiB; depth buffers may add about 4 MiB plus alignment/driver overhead. Staging can retain extra chapter caches temporarily. Fixed strip/shadow geometry and reusable textures aim for 60 FPS, but this is not a measured result. Debug frame statistics record mean/max elapsed frame gaps and turn duration; they cannot prove GPU performance.

Before declaring this milestone ready: build/link with recorded VitaSDK versions, install a VPK, inspect single/spread forward/reverse curls and odd final pages, test rapid controls and interruption states, calibrate touch, verify audio speaker/headphone/mute behavior, restart to verify settings/positions, and record frame pacing, peak RAM, render-target allocation, capture and storage latency. No native compile, on-device visuals, touch/audio result or 60 FPS claim is made here.

## Changed files

- App/config: `src/app/app.hpp`, `app.cpp`, `config.hpp`.
- State/navigation/settings: `src/reader/transition.hpp/.cpp`, `turn_transaction.hpp/.cpp`, `settings.hpp/.cpp`.
- Touch/platform: `src/input/input.hpp`, `touch.hpp/.cpp`, `src/platform/vita.hpp/.cpp`, `frame_stats.hpp`.
- Rendering: `src/rendering/renderer.hpp/.cpp`, `curl.hpp/.cpp`.
- Audio: `src/audio/events.hpp/.cpp`, `src/platform/vita_audio.hpp/.cpp`.
- Storage: `src/storage/settings.hpp/.cpp`, `atomic_file.hpp`, `position.cpp` (Windows host replacement support).
- Build/tests: `CMakeLists.txt`, `tests/session_test.cpp`, `fixture_epub.cpp`, `interaction_test.cpp`, `tools/fetch_host_test_deps.py`.
- Documentation: `README.md`, `TODO.md`, `docs/ARCHITECTURE.md`, `docs/VALIDATION.md`, this file.

PDF, highlighting, annotations, search, cloud sync and final cover/LiveArea assets are outside this milestone.

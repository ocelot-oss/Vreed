# Validation gate

Do not label milestone 1 hardware-ready until these checks pass with a recorded SDK version, console model and firmware.

- Clean Release build and VPK install through VitaShell.
- Launch with no books; directories created and help displayed.
- Rescan sample and uppercase `.EPUB`; directories named `.epub` ignored.
- Open sample; all French test glyphs, including œ, readable.
- Cross, Circle, D-pad, stick and shoulder controls; first press immediate.
- Traverse all pages and both chapter boundaries in each direction; no missing words.
- Exit a book, reopen and restart application; position retained.
- Corrupt the checkpoint; safe restart at the beginning.
- Truncated ZIP, missing OPF, bad spine, malformed XHTML, encrypted ZIP and oversized chapter: visible error; library or current page remains usable.
- Failed storage writes: visible warning, no crash.
- Repeated chapter navigation and opening/closing books: inspect peak RAM, stable usage and frame pacing.
- Verify system font module availability, rendering initialization, and checkpoint rename replacement on the target SDK/firmware.
- Suspend and resume mid-read; no font/GPU/input corruption. Abrupt termination may retain only the previous successful checkpoint.

No physical Vita is connected to this environment. Hardware results remain unverified.

## Local results — 2026-10-05

- Zig 0.16 portable Windows compiler: layout test built and executed successfully. Checked UTF-8 code point boundaries, line width/height limits, long-token splitting, empty chapters and invalid viewports.
- Reader session, persistence and both test translation units compiled separately without diagnostics.
- Python ZIP/XML checks passed for the generated sample: first uncompressed mimetype entry, expected MIME string and well-formed container, OPF, XHTML and NCX.
- EPUB integration test was authored and compiled, but not linked or executed: native libarchive and Expat development libraries are unavailable here.
- The Vita platform/rendering code and EPUB parser implementation have not been compiled with VitaSDK. A Windows syntax attempt against library discovery encountered Windows' one-argument mkdir API; that module targets Vita's POSIX API and is not part of the portable host tests.
- No VPK or hardware result is claimed.

## Milestone 2 — local results, 2026-10-05

- Portable theme/layout suite executed successfully with Zig 0.16: 162 combinations, UTF-8 boundaries, text anchors, viewport containment, independent gutters, large-font fallback and exact OLED black colors.
- Synthetic-session suite executed successfully: cross-chapter spreads, final spread handling, reverse navigation, repeated reflow anchor retention and rollback after metrics/chapter failures. Visible-page handles retain ownership across cache replacement. This is not a real ZIP/XML test.
- Host preview generator compiled; Windows application-control policy blocked its execution. Visual preview and native visual review are outstanding.
- New Vita rendering/input code is not compiled against VitaSDK. Real EPUB integration and hardware tests remain outstanding.

Additional device checks: switch every mode, vary all typography controls, test single/spread chapters and odd final pages, confirm no gutter overlap, close settings with Circle without leaving the reader, toggle the BOOK ribbon and return to its page, reopen VR1/VR2 positions, and inspect OLED menu/error surfaces for true black. Record memory/frame pacing and visually tune grain/shadows on both Vita display models.

## Milestone 3 — local results, 2026-10-05

The portable interaction suite compiled and ran successfully with Zig 0.16: completion/cancel/flick, touch contact ownership, mirrored curl math, ribbon reversal, persisted settings, audio queue and transactional rapid/spread navigation. Existing layout/session binaries passed; both suites recompiled successfully from current sources, but Windows application-control policy blocked execution of those fresh binaries. Their fresh execution remains outstanding. The real EPUB integration test now compiled, linked and executed successfully against isolated Windows MSYS2 libarchive 3.8.9 and Expat 2.8.5; this supersedes the earlier host-library limitation above.

Native VitaSDK compilation remains outstanding. No VPK, native visual validation, touch/audio hardware result or measured 60 FPS is claimed. See [milestone 3](MILESTONE_3.md) and [device checks](../TODO.md).

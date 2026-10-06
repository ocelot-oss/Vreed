# Validation gate

## Second device review — version 00.04

[Run 37538327111](https://github.com/ocelot-oss/Vreed/actions/runs/37538327111) passed all four portable suites and native Vita packaging for `e077be1cdb1eeae60c9ff1e5439f2ea08d59d2b1`. The downloaded 1460881-byte VPK passed SHA-256, ZIP integrity, SFO version/title and branding asset verification. Changes cover center-start swipes, consistent apostrophe advance, extended SELF permissions and storage diagnostics, and shelf-opening animation. These build/test results do not establish resolution on hardware; see [device checks](DEVICE_REVIEW_2.md).

## Vreed branding — version 00.03

[Run 37536589446](https://github.com/ocelot-oss/Vreed/actions/runs/37536589446) passed all four portable suites and native package generation for `8819192163594ab912b0bca6dfd49dc0d5f9612d`. The 1449653-byte `Vreed.vpk` passed SHA-256 and ZIP integrity checks. Its SFO title is `Vreed`, version `00.03`, and retained title ID `VITARD001`. Packaged icon, LiveArea background/startup/template and library logo were verified against the source assets; icon is indexed PNG at 128×128. The supplied original logo is preserved as `assets/branding/source.png`; display name/version originate in `config/identity.cmake`. Books/settings/positions keep their legacy paths. Bubble/LiveArea appearance and update behavior still require Vita review.

## First device-review update — version 00.02

The user reported satisfactory page audio, animations and reading fluidity for version 00.01, alongside checkpoint errors, missing images, apostrophe spacing and inconsistent touch. Version 00.02 addresses those reports and adds the requested startup music, shelf library and explicit resume bookmark; see [change notes](DEVICE_REVIEW_1.md).

[Run 37526595831](https://github.com/ocelot-oss/Vreed/actions/runs/37526595831) passed all four portable suites and native VitaSDK build/package generation for `5b09869e06a335145c247bf95bf2e12d6a58dc0e`. New regression cases cover repeated VR3 checkpoint overwrites/pinned-state round trips, music start/stop, CLEAN touch edges, relative PNG resources, fitted image geometry and elisions across styled runs/nonbreaking whitespace. The downloaded 1130927-byte VPK matched its SHA-256. Toolchain remains release 2026.08, ARM GCC 15.2.0 and CMake 3.28.3. The new native file backend, image output, shelf visuals, ribbon resume behavior and touch changes still require the user's device review.

## GitHub Actions results — 2026-10-06

[Run 37448672300](https://github.com/ocelot-oss/Vreed/actions/runs/37448672300) succeeded for commit `f6e738f4eb61467d3a578a921e5358936f18e16a`. All four portable suites (layout, session, interaction, real EPUB) executed successfully on Ubuntu. Native compilation, linking, SELF and VPK generation succeeded with VitaSDK release 2026.08, sequence 3, toolchain `sdk-core-2026.08.0`, package snapshot `packages-2026.08-snapshot-20260813.3.1`, ARM GCC 15.2.0 and CMake 3.28.3.

The generated 837005-byte VPK was downloaded to local `build/VitaReader.vpk` and verified against the artifact's SHA-256. This supersedes earlier native-build and fresh-host-execution limitations below. Vita installation, display/touch/audio behavior and performance remain unverified.

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

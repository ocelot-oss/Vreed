# VitaReader remaining work

## Validation gates

- Native build/link passed on GitHub Actions with recorded VitaSDK versions; see `docs/VALIDATION.md`.
- Produce and install a VPK; validate all three milestones on real hardware.
- Check PGF font sizes, French accents, per-page clipping, spread readability and true-black output.
- Inspect native BOOK/CLEAN/OLED/overlay and curl visuals.
- Measure draw calls, memory, frame pacing, chapter loads and checkpoint latency on Vita.
- Run real ZIP/XML integration tests with libarchive and Expat, including malformed inputs.

## Deferred features

- Persisted bookmarks and bookmark navigation; settings persistence is implemented.
- Licensed book font, styled typography and text shaping.
- PDF, highlighting, annotations and search.
- Cover thumbnails and final LiveArea assets.
- Suspend/resume validation and power-loss persistence guarantees.

## Milestone 3 device checks

- Verify front-panel ranges, edge hit zones, slow/fast drags, reverse drags and multi-touch cancellation.
- Tune curl strips, backside, gutter handoff and shadows in both directions; inspect odd final spreads.
- Measure the two render targets' actual GPU memory and capture latency; confirm stable 60 FPS animation or reduce quality using the centralized constants.
- Verify audio worker startup/shutdown, volume/mute, speaker/headphone output and silent fallback.
- Test Triangle, Circle, START and suspend during automatic, dragged, accepted and cancelling turns. The current interruption safeguard detects frame gaps; actual focus/suspend behavior needs device testing.
- Check persisted settings after restart, corrupted settings and storage failures.

# Light library and bookmark screenshots — 00.07

The library now uses a warm light background, darker labels and a faint enlarged mascot behind its wooden shelves. The renderer samples the mascot portion of the existing logo texture, preserving the supplied branding asset. Opening animations capture this same library scene.

BOOK displays one ribbon: the top-right corner of the single page, or the right sheet in a spread. Pulling it retains the existing resume anchor semantics (the first page of a visible spread), then renders the current reading view into a reused 960×544 target with the ribbon lowered. GPU completion is awaited before encoding its linear RGBA pixels, respecting byte stride, as a PNG. Images and both visible pages are included; settings/error overlays are excluded.

Captures live in `ux0:/data/Vreed/screenshots/`. Startup and writes create parent directories. The filename combines the book path hash, chapter/page, device timestamp and a collision counter; existing images are not overwritten. Writes use a temporary file followed by replacement. Retracting a bookmark leaves prior captures intact. A saved bookmark remains valid if screenshot storage fails, and the UI reports that partial success. Encoding/writing is performed only on an explicit bookmark action; its device latency remains to be measured.

Hardware checks: inspect the light shelf/mascot contrast with empty/full libraries; verify the left-sheet ribbon is absent and right ribbon animates in both layouts; pull bookmarks with illustrated/text pages, inspect the resulting PNG orientation/colors/text and repeat without losing earlier captures. Check bookmark resume after restart, capture failure reporting and free-storage behavior.

# PDF navigation — 00.11

Landscape PDF content uses 8-pixel side/top margins and starts in width-fit. Tall pages extend below the viewport: drag vertically or use Up/Down to pan; Cross toggles whole-page fit. Portrait layout and default page-fit remain unchanged. BOOK spread still displays two physical pages.

PDF turns bypass curl/slide and their two render-target captures. A per-document LRU keeps at most three recent raster images within 16 MiB, reducing repeated MuPDF rendering on revisits. The first render of a complex uncached page can still take time; no hardware latency measurement is claimed.

PDF-specific touch handling recognizes completed horizontal swipes and short taps in the left/right 28% side zones. Vertical drags pan and cannot also turn a page. Cancellation, changed contacts and overlays reset the gesture. This avoids beginning a blocking page load halfway through a PDF drag. EPUB gesture behavior is unchanged.

Tests cover recent-page identity reuse with real MuPDF, landscape bounds, unchanged portrait geometry, PDF taps/swipes/panning and cancellation. On Vita, check scanned and vector PDFs, fast repeated forward/backward navigation, portrait rotations, spreads, edge taps, vertical panning, L/R and L+R chooser. Check memory behavior across many pages and ensure footer/ribbon remain usable.

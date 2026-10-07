# Version 00.09 device checks

- Library: check the larger logo, beige background without an opaque square, no duplicate Vreed header, and no overlap with the first shelf.
- Sans: open the same French EPUB, change text size repeatedly and switch between Serif, Sans and Mono. Inspect glyph edges, apostrophes and wrapped lines at every size.
- PDF: copy portrait, landscape, scanned and illustrated PDFs into ux0:/data/VitaReader/books and refresh. Open, turn forward/backward and reopen at the saved position.
- PDF controls: Cross toggles whole-page and full-width fit; Up/Down moves vertically. Verify both spread pages, L+R selection, Square bookmark and optional PNG capture.
- Check large PDF loading latency and repeated PDF/EPUB opening for memory failures. The renderer limits each page raster to 1024 by 1400 pixels and rejects password-protected files.

Portable tests and native builds cannot establish font appearance, GPU behavior or PDF performance on actual Vita hardware.

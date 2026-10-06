# VitaReader

Native PS Vita EPUB reader, milestones 1–3. C++17, VitaSDK, vita2d system PGF fonts, libarchive ZIP and Expat XML. Targets 960×544. No browser runtime.

**Status: source implementation; Vita compilation and real hardware validation outstanding. No VPK has been produced in this environment.**

The repository was empty. The available Windows environment had no VitaSDK, CMake, C++ compiler or installed WSL distribution. Public upstream headers and package recipes were inspected rather than assuming APIs.

See [build instructions](docs/BUILD.md), [architecture and support limits](docs/ARCHITECTURE.md), and [hardware acceptance checklist](docs/VALIDATION.md).

Copy EPUB files into `ux0:/data/VitaReader/books`. The application creates its directories on first launch. Discovery is non-recursive, limited to 512 files, with case-insensitive extensions and filename titles until a book is opened. Triangle rescans.

| Control | Library | Reader |
|---|---|---|
| D-pad / left stick | Up / down selection | Left / right pages |
| Cross | Open | — |
| Circle | Dismiss error | Close settings, or save and return |
| L / R | — | Previous / next page or spread; adjust selected setting |
| Triangle | Rescan | Open / close reading settings |
| Square | — | Toggle temporary bookmark in BOOK |
| START | Quit | — |

First press is immediate; held directions repeat after 350 ms then every 110 ms. In settings, up/down selects a row and left/right changes its value. Settings apply immediately. Circle closes the overlay; Circle again returns to the library.

Position is saved after navigation, setting changes and on return. New checkpoints retain a chapter and text offset, so typography changes keep the passage in view. Old milestone-1 checkpoints remain readable. Changing the EPUB content can still shift the resumed location.

## Reading experiences

- **BOOK**: warm paper on a dark surround, restrained grain and edge shadows, running title and folios. Choose one page or an open spread with independent margins and a shaded gutter. Navigation advances by two pages in a spread, including across chapter boundaries. The right sheet is blank at an unmatched final page.
- **CLEAN**: off-white full-screen reading, generous margins, no texture, physical gutter or ribbon.
- **OLED NIGHT**: true-black canvas, subdued off-white text, dim progress, black settings and error surfaces.

Triangle opens an overlay for mode, BOOK layout, font size, line spacing, margins, animation, page sound and volume. Large type falls back to one page rather than shrinking the text to force a spread. The requested layout is restored when choosing a smaller font. Settings persist across launches in `ux0:/data/VitaReader/settings.cfg`; malformed files use safe defaults.

Square toggles one temporary bookmark on the current BOOK page. Its notched red ribbon enters/exits over 220 ms, follows the marked text when repaginating and resets when opening a book. Bookmarks remain temporary.

BOOK defaults to a 360 ms perceptual page curl; CLEAN and OLED NIGHT use a 160 ms slide. Choose NONE for instant turns. BOOK also supports front-touch drags starting at an outer page edge: drag horizontally and release to complete or return. Spread turns advance two pages. Rapid physical presses finish the accepted turn before starting the next; cancelled drags never change the saved position.

Optional quiet paper sounds use original procedural placeholder samples and a dedicated Vita audio worker. They accompany accepted turns and ribbon actions. Four volume levels include mute; sound failure leaves reading usable. Native audio, touch calibration, GPU output and frame pacing remain unvalidated.

See [milestone-3 implementation and test notes](docs/MILESTONE_3.md), [milestone-2 notes](docs/MILESTONE_2.md) and [remaining work](TODO.md).

PDF, highlighting, annotations, search and cover thumbnails remain deferred. Native font quality, French glyph coverage and scene performance still need real Vita validation.

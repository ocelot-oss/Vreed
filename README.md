# Vreed

Version 00.03 adopts the supplied Vreed logo for the library, Vita bubble and LiveArea. Display name/version are centralized in `config/identity.cmake` and generated into the native header. The title ID `VITARD001` and legacy `ux0:/data/VitaReader/` storage remain unchanged so this update retains existing books, settings and resume points. The package is now `Vreed.vpk`.

Native PS Vita EPUB reader, milestones 1–3. C++17, VitaSDK, vita2d system PGF fonts, libarchive ZIP and Expat XML. Targets 960×544. No browser runtime.

**Status: VitaSDK compilation and four portable test suites passed on GitHub Actions on 2026-10-06. A VPK was produced. Installation and real hardware validation remain outstanding.**

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

Triangle includes **Police** (System, Serif, Sans, Mono) and **Capture sur signet**, disabled by default. Font changes preserve the text anchor. Press **L + R together** to choose a page of the whole book: left/right moves one page, up/down ten, or use the touch slider; Cross confirms and Circle cancels. The light shelf background now contains three slowly moving mascot situations. See [00.08 device checks](docs/DEVICE_REVIEW_5.md).

Body paragraphs use justified wrapped lines; headings and final paragraph lines remain aligned left. Version 00.06 also uses buffered stereo playback for the supplied library music. See [audio and alignment review](docs/DEVICE_REVIEW_3.md).

- **BOOK**: warm paper on a dark surround, restrained grain and edge shadows, running title and folios. Choose one page or an open spread with independent margins and a shaded gutter. Navigation advances by two pages in a spread, including across chapter boundaries. The right sheet is blank at an unmatched final page.
- **CLEAN**: off-white full-screen reading, generous margins, no texture, physical gutter or ribbon.
- **OLED NIGHT**: true-black canvas, subdued off-white text, dim progress, black settings and error surfaces.

Triangle opens an overlay for mode, BOOK layout, font size, line spacing, margins, animation, page sound and volume. Large type falls back to one page rather than shrinking the text to force a spread. The requested layout is restored when choosing a smaller font. Settings persist across launches in `ux0:/data/VitaReader/settings.cfg`; malformed files use safe defaults.

Square pulls the BOOK ribbon and saves an explicit resume point, confirmed by a short chime only after successful storage. That point survives further navigation and application restarts. Square on the marked page retracts it and restores automatic resume; Square elsewhere moves it to the current page. The single ribbon keeps a visible retracted tip. In a spread, Square marks the left page.

Only the right-hand ribbon appears in a spread. Captures are disabled by default from 00.08. When Capture sur signet is enabled in the menu, pulling the ribbon also saves a clean 960×544 PNG of the current reading view (both pages in spread mode) into `ux0:/data/Vreed/screenshots/`, created automatically. Files include a book identifier, chapter, page, timestamp and collision counter; repeated captures preserve previous images. Retracting the ribbon retains captures. A screenshot failure reports an error while retaining a successfully saved resume point.

BOOK defaults to a 360 ms perceptual page curl; CLEAN and OLED NIGHT use a 160 ms slide. Choose NONE for instant turns. All modes support front-touch drags starting anywhere on the page: drag horizontally and release to complete or return. Spread turns advance two pages. Rapid physical presses finish the accepted turn before starting the next; cancelled drags never change the saved position.

The library displays books as colored spines on wooden shelves. Touch a spine to select it, then again to open. The supplied recording loops in the library, fades out when a book opens and restarts on return. Existing sound and volume settings control playback. Embedded PNG/JPEG illustrations are fitted into pages; vector SVG, GIF/WebP and CSS backgrounds remain unsupported. Opening a selected book animates it out of the shelf and reveals the reader. See [second device-review changes](docs/DEVICE_REVIEW_2.md).

Optional quiet paper sounds use original procedural placeholder samples and a dedicated Vita audio worker. They accompany accepted turns and ribbon actions. Four volume levels include mute; sound failure leaves reading usable. Native audio, touch calibration, GPU output and frame pacing remain unvalidated.

See [milestone-3 implementation and test notes](docs/MILESTONE_3.md), [milestone-2 notes](docs/MILESTONE_2.md) and [remaining work](TODO.md).

PDF, highlighting, annotations, search and cover thumbnails remain deferred. Native font quality, French glyph coverage and scene performance still need real Vita validation.

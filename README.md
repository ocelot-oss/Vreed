# Vreed

A native EPUB and PDF reader for homebrew-enabled **PS Vita** systems. Browse a bookshelf, read on warm paper or a true-black OLED theme, and keep your place with a bookmark ribbon.

**Current version: 00.10 beta.** The seven portable test suites and native Vita build passed [GitHub Actions](https://github.com/ocelot-oss/Vreed/actions/runs/37753468103). Portrait rendering, PDF comfort and animation performance still need device feedback; this is a beta release.

## Download and install

1. Open [Releases](https://github.com/ocelot-oss/Vreed/releases) and download **Vreed.vpk** from the 00.10 beta release.
2. Copy the VPK to your Vita and install it with **VitaShell**.
3. Launch Vreed once. It creates `ux0:/data/VitaReader/books/` automatically.
4. Copy your `.epub` and `.pdf` files into that directory using VitaShell USB or FTP, then press **Triangle** in the library to refresh.

Updates retain the title ID `VITARD001` and the existing `ux0:/data/VitaReader/` data directory, preserving books, settings and saved positions. The legacy directory name is intentional. Discovery scans the top level of the books directory, supports case-insensitive extensions and is limited to 512 files.

## Reading

- **BOOK:** warm paper, page curl, a running title and a bookmark ribbon; one page or a two-page spread in landscape.
- **CLEAN:** an off-white page with generous margins.
- **OLED NIGHT:** a true-black background with subdued text.
- **Portrait:** press **Triangle → Orientation → Vertical droite / Vertical gauche**, then rotate the Vita. EPUB text reflows into a 544×960 single page; PDF pages keep their original layout. Touch and directional controls follow the orientation. L/R keep their previous/next roles. Returning to horizontal restores your spread preference.
- **Fonts:** System, Serif, Sans and Mono, with adjustable size, spacing and margins. Wrapped body paragraphs are justified; headings and final paragraph lines remain left-aligned.
- **PDF:** press **Cross** to switch between page-fit and width-fit, then use **Up/Down** to pan a magnified page. PDFs retain their original fonts and illustrations.

The library shows colored book spines, moving mascot poses and an opening-book animation. Original music by the project author plays in the library and fades when reading begins. Page sounds, animation style and volume are adjustable.

## Controls

| Control | Library | Reader |
|---|---|---|
| D-pad / left stick | Select a book | Previous/next page; Up/Down pans PDFs |
| Cross | Open the selected book | Toggle PDF zoom; confirm page selection |
| Circle | Dismiss an error | Close an overlay, or save and return to the library |
| L / R | — | Previous / next page or spread; adjust a setting |
| L + R together | — | Open the whole-book page chooser |
| Triangle | Refresh the library | Open / close reading settings |
| Square | — | Save / move / retract the BOOK bookmark |
| Front touch | Tap to select, tap again to open | Swipe to turn pages; drag the page chooser slider |
| START | Quit | — |

In reading settings, Up/Down selects a row and Left/Right changes its value. Settings apply immediately and persist across launches. In portrait, directional controls follow the rotated page. The page chooser moves one page with Left/Right or ten with Up/Down; Cross confirms and Circle cancels.

## Bookmarks and screenshots

Square pulls the BOOK ribbon and saves an explicit resume point, with a confirmation chime after successful storage. That point survives further navigation and application restarts. Square on the marked page retracts it and restores automatic resume; Square elsewhere moves it. Only the right-hand ribbon appears in a spread, while the saved position identifies the spread's first page.

**Capture sur signet is disabled by default.** Enable it in reading settings to save a PNG when placing a bookmark. Captures go into `ux0:/data/Vreed/screenshots/`, created automatically: 960×544 in landscape, or upright 544×960 in portrait. Existing captures are retained when a bookmark is removed. A screenshot failure does not discard a successfully saved bookmark.

Without an explicit bookmark, Vreed saves progress after navigation, setting changes and returning to the library. EPUB positions retain a text anchor across typography and orientation changes. Changing a book's contents can still move its resumed position.

## Supported formats and limits

EPUB supports chapter navigation, justified text and embedded PNG/JPEG illustrations. SVG, GIF/WebP, CSS backgrounds and full browser-style EPUB layout are not supported. PDF rendering uses MuPDF; password-protected PDFs are not supported. Highlighting, annotations, search and cover thumbnails are not implemented.

Please report problems through [Issues](https://github.com/ocelot-oss/Vreed/issues), including your Vita model, Vreed version, book format and steps to reproduce. See the [portrait device checklist](docs/DEVICE_REVIEW_7.md) and [validation record](docs/VALIDATION.md) for checks still requiring hardware feedback.

## Build and sources

Vreed is C++17 with VitaSDK and vita2d, with libarchive/Expat for EPUB and MuPDF for PDF. No browser runtime is used. See [build instructions](docs/BUILD.md) and [architecture](docs/ARCHITECTURE.md).

Each release includes **Vreed-Sources.tar.gz**, containing the corresponding application sources, build recipes, pinned MuPDF engine and dependencies. Use that archive for the exact sources accompanying a VPK. GitHub's automatically generated source ZIP alone does not include the downloaded MuPDF dependency tree. Actions artifacts are temporary; published release assets provide the downloads for users.

## License and credits

Vreed is distributed under **AGPL-3.0-or-later**. See [LICENSE](LICENSE) and [third-party notices](THIRD_PARTY_NOTICES.md). MuPDF is from Artifex; the patched vita2d font renderer retains its MIT license, and DejaVu fonts retain their original license.

“Mascot's Library Visit” is original music by the project author, included with their permission. The Vreed mascot and branding were supplied by the project author; additional mascot poses and the transparent library logo were prepared with image generation tools. Asset provenance is documented in `assets/branding/` and `assets/audio/`.

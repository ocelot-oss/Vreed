# First Vita review — version 00.02

The user's console review confirmed that page sounds, animations and reading fluidity were satisfactory. Reported issues were repeated checkpoint errors, missing illustrations, spaces after apostrophes and inconsistent touch. Requested additions were an abstract elevator-style startup tune, a bookshelf library and a persistent pull-out bookmark.

## Changes

- Library uses two wooden shelves with 20 colored spines per view, titles and a selected-book border. Physical directions move between books; touch selects a spine and a second tap opens it.
- Original procedural six-second startup music plays once, at the current sound setting/volume. Opening a book stops it. No recording or external music was used.
- Native settings/checkpoints use `sceIoOpen/Read/Write/Close`, with partial-write handling. Vita replacement uses a backup when renaming over an existing checkpoint fails; reads recover the backup if the main file is missing. Host writes remain portable. This addresses two plausible causes in the old file path; actual error resolution must be confirmed on the console.
- VR3 checkpoints preserve a manually selected resume anchor. Square pulls the BOOK ribbon out and saves the page. Confirmation plays only after successful storage. Subsequent turns and returning to the library retain that explicit resume anchor. Square on the marked page retracts it and restores automatic resume; Square on another page moves it there. VR1/VR2 checkpoints remain readable.
- Each BOOK sheet shows the retracted ribbon tip. In a spread, Square marks the left/current page, whose ribbon extends. Reopening/restaring resumes the marked passage.
- Embedded PNG/JPEG illustration resources are resolved relative to chapter paths, checked for dimensions, inserted into pagination and fitted with their aspect ratio preserved. Images inside SVG wrappers referencing PNG/JPEG are recognized; standalone vector SVG, GIF, WebP and CSS backgrounds remain unsupported. Entry size remains 4 MiB, chapter image bytes are capped at 8 MiB, dimensions at 2048×2048, and cached image textures at 24 MiB. Oversize, broken or unsupported resources show a readable fallback. PNG decoding uses a bounded libpng memory reader. Textures are prepared outside animated frames and reused in captured turn surfaces.
- Whitespace following common French elision prefixes (`L'`, `d’`, `qu’`, etc.) is removed across XML text callbacks and inline emphasis boundaries. Meaningful whitespace elsewhere remains unchanged.
- Touch edge zones expand to 96 px and include outer screen margins. CLEAN/OLED also accept gestures. An empty touch-poll result no longer cancels an active finger. Multiple-contact and contact-ID safety rules remain in force.

## Validation

Added tests cover repeated checkpoint overwrite, persisted VR3 bookmark flags, intro start/stop, touch edges in CLEAN, relative PNG resources, bounded image placement, invalid images and French apostrophes across styled runs. The redistributable test EPUB now includes an original PNG and elision examples.

Device acceptance: install the updated VPK, turn multiple pages without a save error, restart with a pulled bookmark and confirm the selected passage, retract it and confirm automatic resume, inspect illustrations in single/spread/CLEAN/OLED, repeat left/right gestures and inspect startup sound, shelves and ribbon visuals. Build/test results are recorded in `VALIDATION.md` after CI completes. Hardware behavior is not inferred from build success.

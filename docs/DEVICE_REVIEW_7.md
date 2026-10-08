# Version 00.10: portrait reading

Triangle opens reading settings; the last row, Orientation, offers Horizontal, Vertical droite and Vertical gauche. Portrait uses a 544×960 logical viewport and one page in BOOK, CLEAN and OLED NIGHT. EPUB reflow retains the text anchor; horizontal mode restores the requested spread. PDF fit and width zoom use the new content rectangle. Settings VRS3 stores the orientation and accepts older VRS1/VRS2 files with horizontal defaults.

The renderer rotates the existing vita2d projection without allocating another frame-sized render target. Page turns, opening animation, clipping, overlays, ribbon and messages share this coordinate system. Screen captures rotate pixels back into upright portrait PNGs. Front touch and D-pad directions follow the chosen orientation; L/R and the L+R chord keep their existing roles.

Portable orientation regressions exercise viewport bounds, spread fallback/restoration, curl texture coordinates, touch/button remapping, persistence and legacy settings migration. Native GPU output still requires device review.

On Vita, test both orientations with each theme and font. Turn pages using swipes in both directions and L/R; open the L+R chooser, drag its slider and confirm. Rotate mid-chapter and confirm the passage remains visible. Open a PDF, toggle width zoom and pan. Enable Capture sur signet and check an upright 544×960 PNG. Return to the shelf and reopen, then restart the app and verify the orientation. Inspect curl, slide, shelf opening, clipped glyphs and OLED settings for visual regressions.

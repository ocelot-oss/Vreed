# Milestone 2 — reading identity

The existing architecture is extended. VitaSDK remains unavailable: no native build, VPK, hardware compatibility or on-device visual polish is claimed.

`reader/settings` describes themes, typography choices and page geometry. `rendering/page_style` supplies cheap rectangle-based paper decoration and an independent bookmark ribbon. The Vita renderer consumes these descriptions; EPUB parsing is unchanged.

## Presentation

BOOK single uses a centered 816 × 506 sheet. A spread uses two 450 ×rff 500 sheets meeting at x=480. Each has its own margins and clipping rectangle; text cannot enter the gutter. Layered alpha strips suggest curvature and shadows. Sparse deterministic grain and thin edge highlights need no texture assets, shaders or curved geometry. Paper and ribbon geometry are cached on presentation changes.

CLEAN uses an off-white full canvas without physical decoration. OLED uses opaque RGB (0,0,0) for the canvas and paper, including the settings sheet and error strip. Text is subdued rather than full white. A quiet running title and chapter/page progress replace a permanent toolbar.

All modes retain the system PGF font with three scales, three line-spacing choices and three margins. Baseline spacing uses measured heading height plus the selected gap. Large type disables the spread while retaining the preference, restoring it when type becomes smaller. Font coverage and readable sizes require Vita evaluation; a packaged book font remains future work.

## Reflow and spreads

Pagination records offsets into each chapter's intermediate text at UTF-8 boundaries. Reflow locates the page containing the same anchor and retains it through repeated setting changes. Navigation resets the anchor to the new page start. VR2 checkpoints store this anchor; VR1 checkpoints still load by chapter/page. Changes to book content can invalidate offsets.

The session retains the current chapter and one adjacent cache. The second visible page may belong to the next chapter. Spread turns stage their destination before committing; a failed chapter load cannot leave a half-completed turn. Once the final page is visible, forward navigation stays put. Odd final pages leave a blank right sheet. Folios use chapter/page coordinates; no global page count or book-wide pagination pass is invented.

The app refreshes the right-page view only after opening, navigation or reflow. Unchanged frames do not reopen archives or repaginate. A failed right-page load preserves the current page and displays an error; another navigation/settings action can retry.

## Settings and ribbon

Triangle toggles the overlay. Up/down selects one of five rows; left/right or L/R changes it. Circle closes the overlay before returning to the library. BOOK layout is disabled outside BOOK. A note explains large-type fallback. Changes apply immediately and last for the current run.

Square toggles one temporary BOOK bookmark on the current text. It survives navigation within the open book and is shown when its anchor is on the current page. The separately generated ribbon sits on the upper outer edge of the sheet or right sheet of a spread, ready for later animation. Opening a book resets it. No bookmark database is introduced.

## Host review

Portable layout/theme and synthetic-session tests were executed successfully. The synthetic provider tests navigation and failure handling independently of the real ZIP/XML parser. Real EPUB integration remains unexecuted; host libarchive/Expat are absent. Controller and rendering changes are not compiled against VitaSDK.

`tools/preview_reading.cpp` exports four SVG studies using the same portable geometry, decoration and pagination, with approximate host font metrics. It compiled, but Windows application-control policy blocked execution. No generated preview or Vita screenshot is claimed. On a host that can run it:

```sh
c++ -std=c++17 -Isrc tools/preview_reading.cpp src/reader/layout.cpp src/reader/settings.cpp src/rendering/page_style.cpp -o preview_reading
./preview_reading docs/previews
```

The optional `tools/render_previews.cjs` uses sharp to rasterize the SVGs and create a comparison sheet. Host rendering cannot validate PGF glyphs, clipping, paper grain visibility or Vita performance.

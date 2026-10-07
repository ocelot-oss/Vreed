Based on xerpi/libvita2d a8f15ab09d5233f0a4e4ad0e8f6ade0da888cbed (MIT).
Vreed caches glyphs separately at each pixel size and adds two transparent pixels around every glyph to prevent adjacent glyph bleed. Linear magnification remains available for the shared system-font atlas.

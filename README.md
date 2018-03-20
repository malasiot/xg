# A C++ 2D graphics api and SVG renderer.

Basically a high level wrapper over cairo but designed to allow extension to other backends.

Also basic SVG 1.1 rendering support (suitable for rendering symbols or icons). 

SVG support includes:
+ All basic shape rendering: rect, circle, ellipse, line, polyline, polygon, path
+ Coordinate transformation and viewport handling
+ Full paint server support: solid color, linear gradients, radial gradients, patterns
+ Image support
+ Container support: svg, g, use, symbol
+ Clipping: not supporting ORing of clipping shapes.
+ Styling: presentation attributes only
+ Rudimentary text rendering: font selection, shaping with Hurfbuzz
..+ Only single x, y, dx, dy attribute handled.
..+ text-anchor works only for simple text (not sequence of TSpans)
..+ No vertical text
+ No masks, filters, markers and group opacity.

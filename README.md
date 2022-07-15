# ShittyGUI
A super simple GUI library for semi-embedded use cases. Graphics output is to a bitmapped framebuffer, and events to drive the interface are ingested through various event sources. UI components are designed for touch interaction, with support for auxiliary input devices such as rotary encoders or physical buttons.

## Dependencies
CMake (at least 3.18) is required to build the library. In addition, the following libraries are mandatory for use with the library:

- [Cairo](https://www.cairographics.org): Main graphics library, used to implement drawing of controls
- [FreeType](https://freetype.org): Font rasterization
- [Pango](https://pango.gnome.org): Text layout and rendering
- [HarfBuzz](https://harfbuzz.github.io): Text shaping

If present, the following libraries can be used to provide enhancements to the library:

- fontconfig: Automatic detection and loading of the system's fonts.
- [libpng](http://libpng.org/pub/png/libpng.html): Loading of PNG formatted bitmaps for image rendering


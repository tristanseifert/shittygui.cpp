# ShittyGUI
A super simple GUI library for semi-embedded use cases, where an entire screen is controlled directly. Graphics output is to a bitmapped framebuffer, and events to drive the interface are ingested through various event sources. UI components are designed for touch interaction, with support for auxiliary input devices such as rotary encoders or physical buttons.

## Design
ShittyGUI is designed to be as simple as possible to integrate. To get started, you need nothing more than some memory region for the library to render into: its framebuffer. With a framebuffer, you can create a `shittygui::Screen` which serves as the primary object of interest for the library. Screens are responsible for rendering stuff into the framebuffer, as well as accepting input from various input sources, and dispatching it to the appropriate controls. Each screen has the provision for a single root widget which is used to contain all other widgets; these widgets can be arbitrarily nested.

Internally all objects are stored using smart pointers to alleviate memory lifetime concerns. Widgets will automagically be retained for as long as they're visible on screen.

## Dependencies
CMake (at least 3.18) is required to build the library. ShittyGUI is implemented in C++20, and thus requires a relatively recent compiler (only clang is tested) and feature complete C++ runtime. In addition, the following libraries are mandatory for use with the library:

- [Cairo](https://www.cairographics.org): Main graphics library, used to implement drawing of controls
- [FreeType](https://freetype.org): Font rasterization
- [Pango](https://pango.gnome.org): Text layout and rendering
- [HarfBuzz](https://harfbuzz.github.io): Text shaping

If present, the following libraries can be used to provide enhancements to the library:

- fontconfig: Automatic detection and loading of the system's fonts.
- [libpng](http://libpng.org/pub/png/libpng.html): Loading of PNG formatted bitmaps for image rendering


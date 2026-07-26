# tshow

**tshow** is a lightweight Linux utility that visualizes multitouch input directly on the screen.

It creates a transparent, click-through overlay window above the desktop and renders visual indicators for every active touch point. The overlay is completely non-interactive, allowing all input events to continue reaching the underlying applications.

The project is intended both as a practical debugging tool for touchscreen devices and as a playground for experimenting with Linux input subsystems and graphics.

![touch_visualization](./touch_visualization.gif)


## How it works

`tshow` combines several Linux technologies:

* **libinput** for touch event processing
* **GLFW** for window creation and OpenGL context management
* **NanoVG** for 2D vector rendering
* **X11/XFixes** for creating a transparent, click-through overlay window

The application creates an OpenGL window with an alpha channel, configures it as a non-interactive overlay, listens for multitouch events, and renders visual feedback for each active contact.

For anatomical reasons the number of marker is limited to 10,

## Building

Dependencies: 

* C compiler
* GNU make
* OpenGL
* GLFW
* NanoVG
* X11
* XFixes
* libinput
* libudev

Example for Arch Linux:

```sh
sudo pacman -S base-devel glfw-x11 libinput libx11 libxfixes mesa
```

NanoVG is expected to be available in the project source tree.

To build use the command

```sh
make
```

## Running

```sh
./tshow
```

Depending on your system configuration, access to input devices may require elevated privileges or appropriate udev permissions.

## Architecture

```
             ┌─────────────┐
             │  libinput   │
             └──────┬──────┘
                    │
            Touch Events
                    │
                    ▼
             ┌─────────────┐
             │    tshow    │
             └──────┬──────┘
                    │
      ┌─────────────┴─────────────┐
      │                           │
      ▼                           ▼
 GLFW / OpenGL               NanoVG Renderer
      │                           │
      └─────────────┬─────────────┘
                    ▼
          Transparent X11 Overlay
```

<!-- ## Motivation -->
<!---->
<!-- Linux offers excellent support for touch input through `libinput`, but there are relatively few lightweight tools for visualizing multitouch interactions in real time. -->
<!-- `tshow` aims to provide a simple, responsive overlay that is useful for: -->
<!---->
<!-- * Touchscreen debugging -->
<!-- * Device testing -->
<!-- * Demonstrations -->
<!-- * User interaction recordings -->
<!-- * Educational purposes -->

## TODOs

- [ ] Gesture recognition

## License

This project is licensed under the MIT License.


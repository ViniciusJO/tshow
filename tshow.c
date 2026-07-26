#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
// #include <stdlib.h>
#include <stdbool.h>

#include <GL/gl.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include "nanovg/src/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/src/nanovg_gl.h"


#define RGBA(R,G,B,A) (NVGcolor){ .r = (R)/255.0f, .g = (G)/255.0f, .b = (B)/255.0f, .a = (A)/255.0f }
#define LIGHTGRAY  RGBA(200, 200, 200, 255 )   // Light Gray
#define GRAY       RGBA(130, 130, 130, 255 )   // Gray
#define DARKGRAY   RGBA(80, 80, 80, 255 )      // Dark Gray
#define YELLOW     RGBA(253, 249, 0, 255 )     // Yellow
#define GOLD       RGBA(255, 203, 0, 255 )     // Gold
#define ORANGE     RGBA(255, 161, 0, 255 )     // Orange
#define PINK       RGBA(255, 109, 194, 255 )   // Pink
#define RED        RGBA(230, 41, 55, 255 )     // Red
#define MAROON     RGBA(190, 33, 55, 255 )     // Maroon
#define GREEN      RGBA(0, 228, 48, 255 )      // Green
#define LIME       RGBA(0, 158, 47, 255 )      // Lime
#define DARKGREEN  RGBA(0, 117, 44, 255 )      // Dark Green
#define SKYBLUE    RGBA(102, 191, 255, 255 )   // Sky Blue
#define BLUE       RGBA(0, 121, 241, 255 )     // Blue
#define DARKBLUE   RGBA(0, 82, 172, 255 )      // Dark Blue
#define PURPLE     RGBA(200, 122, 255, 255 )   // Purple
#define VIOLET     RGBA(135, 60, 190, 255 )    // Violet
#define DARKPURPLE RGBA(112, 31, 126, 255 )    // Dark Purple
#define BEIGE      RGBA(211, 176, 131, 255 )   // Beige
#define BROWN      RGBA(127, 106, 79, 255 )    // Brown
#define DARKBROWN  RGBA(76, 63, 47, 255 )      // Dark Brown

#define WHITE      RGBA(255, 255, 255, 255 )   // White
#define BLACK      RGBA(0, 0, 0, 255 )         // Black
#define BLANK      RGBA(0, 0, 0, 0 )           // Blank (Transparent)
#define MAGENTA    RGBA(255, 0, 255, 255 )     // Magenta
#define RAYWHITE   RGBA(245, 245, 245, 255 )   // My own White (raylib logo)

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <libinput.h>
#include <libudev.h>

#define TARGET_FPS 60
#define FADEOUT_SECS 0.5

#define FINGER_MARKER_RADIUS 30

NVGcolor fingers[10] = { ORANGE, WHITE, BLUE, GRAY, PINK, LIME, RED, BROWN, YELLOW, VIOLET };

int open_callback(const char *path, int flags, void *user_data) { (void)user_data; return open(path, flags); }
void close_callback(int fd, void *user_data) { (void)user_data; close(fd); }

typedef struct Touch {
  double x, y;
  bool down, fade;
  size_t fadeout;
} Touch;

typedef Touch Touchs[10];

typedef struct { double x, y; } Vector2;


static void set_clickthrough(Display *display, Window window) {
    XserverRegion region = XFixesCreateRegion(display, NULL, 0);

    XFixesSetWindowShapeRegion(
        display,
        window,
        ShapeInput,
        0,
        0,
        region);

    XFixesDestroyRegion(display, region);
}

static void set_override_redirect(Display *display, Window window) {
    XSetWindowAttributes attr = {
        .override_redirect = True,
    };

    XChangeWindowAttributes(
        display,
        window,
        CWOverrideRedirect,
        &attr);
}

static void set_above(Display *display, Window window) {
    Atom state =
        XInternAtom(display, "_NET_WM_STATE", False);

    Atom atoms[2] =
    {
        XInternAtom(display, "_NET_WM_STATE_ABOVE", False),
        XInternAtom(display, "_NET_WM_STATE_STICKY", False),
    };

    XChangeProperty(
        display,
        window,
        state,
        XA_ATOM,
        32,
        PropModeReplace,
        (unsigned char *)atoms,
        2);
}

static void set_all_desktops(Display *display, Window window) {
    Atom atom =
        XInternAtom(display,
                    "_NET_WM_DESKTOP",
                    False);

    unsigned long desktop = 0xFFFFFFFFUL;

    XChangeProperty(
        display,
        window,
        atom,
        XA_CARDINAL,
        32,
        PropModeReplace,
        (unsigned char *)&desktop,
        1);
}

static void configure_window(GLFWwindow *window) {
    Display *display = glfwGetX11Display();
    Window xwindow = glfwGetX11Window(window);

    set_override_redirect(display, xwindow);
    set_clickthrough(display, xwindow);
    set_above(display, xwindow);
    set_all_desktops(display, xwindow);

    XRaiseWindow(display, xwindow);

    XFlush(display);
}

void draw_circle(NVGcontext* vg, float x, float y, float r, NVGcolor c);
void draw_ring(NVGcontext* vg, float x, float y, float r, float thickness, NVGcolor c);
void draw_line(NVGcontext* vg, float x0, float y0, float x1, float y1, float w, NVGcolor c);
void draw_rect(NVGcontext* vg, float x, float y, float w, float h, NVGcolor c);

int main(void) {

  // NVGcolor fingers[10] = { ORANGE, WHITE, BLUE, GRAY, PINK, LIME, RED, BROWN, YELLOW, VIOLET };

  if (!glfwInit()) return 1;

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "overlay", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  configure_window(window);
  glfwShowWindow(window);

  Display *display = glfwGetX11Display();
  Window win = glfwGetX11Window(window);

  XWindowAttributes attr;
  XGetWindowAttributes(display, win, &attr);
  {
    const int expected_depth = 32;
    if(attr.depth != expected_depth) {
      fprintf(stderr, "ERROR: Failed to request ARGB framebuffer. Expected depth is %d but get %d.\n", expected_depth, attr.depth);
      exit(1);
    }
  }

  NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
  if (!vg) {
    fprintf(stderr, "Cannot create NanoVG context.\n");
    exit(1);
  }

  struct udev *udev = udev_new();
  struct libinput_interface interface = { .close_restricted = close_callback, .open_restricted = open_callback };
  struct libinput *li = libinput_udev_create_context(&interface, NULL, udev);
  libinput_udev_assign_seat(li, "seat0");

  struct libinput_event_touch *touch = NULL;
  struct libinput_event *event;

  Touchs ts = {0};
  Vector2 f[10];

  int w, h;
  int fbw, fbh;
  int count = 0;

  while (!glfwWindowShouldClose(window)) {
    glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &fbw, &fbh);

    float pxRatio = (float)fbw / (float)w;

    libinput_dispatch(li);

    while ((event = libinput_get_event(li))) {
      switch (libinput_event_get_type(event)) {
        case LIBINPUT_EVENT_TOUCH_DOWN: {
          touch = libinput_event_get_touch_event(event); 
          int slot = libinput_event_touch_get_seat_slot(touch);
          if(slot >= 10) break;
          ts[slot].x = libinput_event_touch_get_x_transformed(touch, w);
          ts[slot].y = libinput_event_touch_get_y_transformed(touch, h);
          ts[slot].down = true;
          ts[slot].fade = false;
          ts[slot].fadeout = TARGET_FPS * FADEOUT_SECS;
        } break;
        case LIBINPUT_EVENT_TOUCH_MOTION: {
          touch = libinput_event_get_touch_event(event);
          int slot = libinput_event_touch_get_seat_slot(touch);
          if(slot >= 10) break;
          ts[slot].x = libinput_event_touch_get_x_transformed(touch, w);
          ts[slot].y = libinput_event_touch_get_y_transformed(touch, h);
        } break;
        case LIBINPUT_EVENT_TOUCH_UP: {
          touch = libinput_event_get_touch_event(event);
          int slot = libinput_event_touch_get_seat_slot(touch);
          if(slot >= 10) break;
          ts[slot].down = false;
          ts[slot].fade = true;
        } break;
        default: break;
      }
      libinput_event_destroy(event);
    }

    glViewport(0, 0, fbw, fbh);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    nvgBeginFrame(vg, w, h, pxRatio);

    NVGcolor c = nvgRGBA(0,0,0,0);
    count = 0;

    for(unsigned char i = 0; i < 10; i++) if(ts[i].down || ts[i].fadeout > 0) {
      int radius = FINGER_MARKER_RADIUS;
      const float fadeout_fac = (float)ts[i].fadeout / (float)(TARGET_FPS*FADEOUT_SECS);
      c = WHITE; (void)fingers[i];
      c.a = fadeout_fac * 0.1 * 255;
      draw_circle(vg, ts[i].x, ts[i].y, fadeout_fac * radius, c);
      f[count].x = ts[i].x;
      f[count].y = ts[i].y;

      radius -= 10;

      while(radius > 0) {
        draw_ring(vg, ts[i].x, ts[i].y, fadeout_fac * radius, 5, c);
        radius -= 10;
      }
      count++;
      if(ts[i].fade) ts[i].fadeout--;
    }

    bool debug = false;
    if(debug && count > 1) {
      c = nvgRGBA(255, 255, 255, 255);
      Vector2 sum = { .x = f[0].x/count, .y = f[0].y/count };
      for(unsigned char i = 1; i < count; i++) {
        draw_line(vg, f[i].x, f[i].y, f[i-1].x, f[i-1].y, 8, c);
        sum.x += f[i].x/count;
        sum.y += f[i].y/count;
      }
      if(count > 2) {
        draw_line(vg, f[count-1].x, f[count-1].y, f[0].x, f[0].y, 6, c);
        draw_circle(vg, sum.x, sum.y, 25, nvgRGBA(255, 100, 100, 255));
      }
    }

    nvgEndFrame(vg);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
}


void draw_circle(NVGcontext* vg, float x, float y, float r, NVGcolor c) {
  nvgBeginPath(vg);
  nvgCircle(vg, x, y, r);
  nvgFillColor(vg, c);
  nvgFill(vg);
}


void draw_ring(NVGcontext* vg, float x, float y, float r, float thickness, NVGcolor c) {
  nvgBeginPath(vg);
  nvgCircle(vg, x, y, r);
  nvgStrokeWidth(vg, thickness);
  nvgStrokeColor(vg, c);
  nvgStroke(vg);
}

void draw_line(NVGcontext* vg, float x0, float y0, float x1, float y1, float w, NVGcolor c) {
  nvgBeginPath(vg);
  nvgMoveTo(vg, x0, y0);
  nvgLineTo(vg, x1, y1);
  nvgStrokeWidth(vg, w);
  nvgStrokeColor(vg, c);
  nvgStroke(vg);
}

void draw_rect(NVGcontext* vg, float x, float y, float w, float h, NVGcolor c) {
  nvgBeginPath(vg);
  nvgRect(vg, x, y, w, h);
  nvgFillColor(vg, c);
  nvgFill(vg);
}

// TODO: implement
// void draw_text(NVGcontext* vg, float x, float y, char* text, float w, NVGcolor c) {
//   nvgBeginPath(vg);
//   // nvgMoveTo(vg, x, y);
//   // nvgLineTo(vg, x1, y1);
//   nvgText(vg, x, y, text, NULL);
//   nvgStrokeWidth(vg, w);
//   nvgStrokeColor(vg, c);
//   nvgStroke(vg);
// }












// #include <stdio.h>
// #include <unistd.h>
// #include <fcntl.h>
//
// #include <raylib.h>
// #include <raymath.h>
//
// #include <libinput.h>
// #include <libudev.h>
//
// #define TARGET_FPS 60
// #define FADEOUT_SECS 0.5
//
// Color fingers[10] = { ORANGE, WHITE, BLUE, GRAY, PINK, LIME, RED, BROWN, YELLOW, VIOLET };
//
// int open_callback(const char *path, int flags, void *user_data) { (void)user_data; return open(path, flags); }
// void close_callback(int fd, void *user_data) { (void)user_data; close(fd); }
//
// typedef struct Touch {
//   double x, y;
//   bool down, fade;
//   size_t fadeout;
// } Touch;
//
// typedef Touch Touchs[10];
//
// extern void set_window_attributes(void *window);
//
// int maino(void) {
//   const int screen_width = 1920;
//   const int screen_height = 1080;
//
//   const int window_width = screen_width;
//   const int window_height = screen_height;
//
//   InitWindow(window_width, window_height, "tshow");
//   SetTargetFPS(TARGET_FPS);
//
//
//   void* window = GetWindowHandle();
//   set_window_attributes(window);
//
//
//   struct udev *udev = udev_new();
//
//   struct libinput_interface interface = { .close_restricted = close_callback, .open_restricted = open_callback };
//   struct libinput *li = libinput_udev_create_context(&interface, NULL, udev);
//
//   libinput_udev_assign_seat(li, "seat0");
//
//   Touchs ts = {0};
//
//   struct libinput_event_touch *touch = NULL;
//   struct libinput_event *event;
//
//   while(!WindowShouldClose()) {
//     libinput_dispatch(li);
//
//     while ((event = libinput_get_event(li))) {
//       switch (libinput_event_get_type(event)) {
//         case LIBINPUT_EVENT_TOUCH_DOWN: {
//           touch = libinput_event_get_touch_event(event); 
//           int slot = libinput_event_touch_get_seat_slot(touch);
//           if(slot >= 10) break;
//           ts[slot].x = libinput_event_touch_get_x_transformed(touch, screen_width);
//           ts[slot].y = libinput_event_touch_get_y_transformed(touch, screen_height);
//           ts[slot].down = true;
//           ts[slot].fade = false;
//           ts[slot].fadeout = TARGET_FPS * FADEOUT_SECS;
//           // printf("touch %d down, { %f, %f }\n", slot, ts[slot].x, ts[slot].y);
//
//           // Vector2 p = GetWindowPosition();
//           // printf("window click: { %f, %f }\n", ts[slot].x - p.x + screen_width/2.0, ts[slot].y - p.y + screen_height/2.0);
//           // printf("window position: { %f, %f }\n", p.x, p.y);
//           // printf("window click: { %f, %f }\n", ts[slot].x, ts[slot].y);
//           // printf("window click pos: { %f, %f }\n", ts[slot].x - p.x, ts[slot].y - p.y);
//         } break;
//         case LIBINPUT_EVENT_TOUCH_MOTION: {
//           touch = libinput_event_get_touch_event(event);
//           int slot = libinput_event_touch_get_seat_slot(touch);
//           if(slot >= 10) break;
//           ts[slot].x = libinput_event_touch_get_x_transformed(touch, screen_width);
//           ts[slot].y = libinput_event_touch_get_y_transformed(touch, screen_height);
//           // printf("touch %d move, { %f, %f }\n", position.slot, position.x, position.y);
//         } break;
//         case LIBINPUT_EVENT_TOUCH_UP: {
//           touch = libinput_event_get_touch_event(event);
//           int slot = libinput_event_touch_get_seat_slot(touch);
//           if(slot >= 10) break;
//           ts[slot].down = false;
//           ts[slot].fade = true;
//           // printf("touch %d up, { %f, %f }\n", position.slot, position.x, position.y);
//         } break;
//         default: break;
//       }
//       libinput_event_destroy(event);
//     }
//
//
//     int count = 0;
//     struct {
//       Vector2 pos;
//     } f[10];
//
//     BeginDrawing();
//     {
//       Color c = BLACK;
//       c.a = 0;
//       ClearBackground(c);
//       Vector2 p = GetWindowPosition();
//       for(unsigned char i = 0; i < 10; i++) if(ts[i].down || ts[i].fadeout > 0) {
//         const float fadeout_fac = (float)ts[i].fadeout / (float)(TARGET_FPS*FADEOUT_SECS);
//         c = fingers[i];
//         c.a = fadeout_fac * 255;
//         DrawCircle(ts[i].x - p.x, ts[i].y - p.y, fadeout_fac * 50, c);
//         // c.a = 255;
//         // c.b = 255;
//         c = BLACK;
//         DrawCircleLines(ts[i].x - p.x, ts[i].y - p.y, fadeout_fac * 40, c);
//         DrawCircleLines(ts[i].x - p.x, ts[i].y - p.y, fadeout_fac * 30, c);
//         DrawCircleLines(ts[i].x - p.x, ts[i].y - p.y, fadeout_fac * 20, c);
//         DrawCircleLines(ts[i].x - p.x, ts[i].y - p.y, fadeout_fac * 10, c);
//         f[count].pos.x = ts[i].x - p.x;
//         f[count].pos.y = ts[i].y - p.y;
//         count++;
//         if(ts[i].fade) ts[i].fadeout--;
//       }
//
//       if(count > 1) {
//         Vector2 sum = Vector2Scale(f[0].pos, 1.0/count);
//         for(unsigned char i = 1; i < count; i++) {
//           DrawLineEx(f[i].pos, f[i-1].pos, 5, WHITE);
//           sum = Vector2Add(sum, Vector2Scale(f[i].pos, 1.0/count));
//         }
//         if(count > 2) {
//           DrawLineEx(f[count-1].pos, f[0].pos, 5, WHITE);
//           DrawCircleV(sum, 25, MAGENTA);
//         }
//       }
//
//     }
//     EndDrawing();
//   }
//
//   return 0;
// }







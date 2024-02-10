#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

Display* display;
Window root;
int screen;
static unsigned int win_focus;
static unsigned int win_unfocus;

unsigned long getcolor(const char* color) {
    XColor c;
    Colormap map = DefaultColormap(display,screen);

    if(!XAllocNamedColor(display,map,color,&c,&c)){
        printf("Error parsing color!");
        exit(1);
    }

    return c.pixel;
}


void Frame(Window w, bool was_created_before_window_manager){
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;
    XWindowAttributes x_window_attrs;
    XGetWindowAttributes(display, w, &x_window_attrs);
    if (was_created_before_window_manager) {
    if (x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable) {
      return;
    }
    }
    const Window frame = XCreateSimpleWindow(
      display,
      root,
      x_window_attrs.x,
      x_window_attrs.y,
      x_window_attrs.width,
      x_window_attrs.height,
      BORDER_WIDTH,
      BORDER_COLOR,
      BG_COLOR);
    XSelectInput(
      display,
      frame,
      SubstructureRedirectMask | SubstructureNotifyMask);
    XAddToSaveSet(display, w);
    XReparentWindow(
      display,
      w,
      frame,
      0, 0);
    XMapWindow(display, frame);
    fprintf(stderr, "framed window\n");
}

void mapWindow(XEvent ev){
    /*const unsigned int BORDER_WIDTH = 3;
    //const unsigned long BORDER_COLOR = 0x000000;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;
    XWindowAttributes x_window_attrs;
    XGetWindowAttributes(display, ev.xmaprequest.window, &x_window_attrs);
    const Window frame = XCreateSimpleWindow(
      display,
      root,
      x_window_attrs.x,
      x_window_attrs.y,
      x_window_attrs.width,
      x_window_attrs.height,
      BORDER_WIDTH,
      BORDER_COLOR,
      BG_COLOR);
    XSelectInput(
      display,
      frame,
      SubstructureRedirectMask | SubstructureNotifyMask);
    XAddToSaveSet(display, ev.xmaprequest.window); //Add client to save set, so that it will be restored and kept alive if we crash.
    XReparentWindow(
      display,
      ev.xmaprequest.window,
      frame,
      0, 0);
    //XMapWindow(display, ev.xmaprequest.window);
    XMapWindow(display, frame);
    fprintf(stderr, "MAPPED WINDOW");
    XSetWindowBorderWidth(display, ev.xmaprequest.window, 1);
    XSetWindowBorder(display, ev.xmaprequest.window, win_focus);
    XWindowAttributes attr;
    Pixmap pixmap = XCreatePixmap(display, ev.xmaprequest.window, );
    attr.border_pixmap = XCreatePixmap()
    XChangeWindowAttributes(display, ev.xmaprequest.window, 0, );
    XRaiseWindow(display, ev.xmaprequest.window);
    XMapWindow(display, ev.xmaprequest.window);*/
    Frame(ev.xmaprequest.window, false);
    XMapWindow(display, ev.xmaprequest.window);
}


int main(){
    //XWindowAttributes attr;
    //XButtonEvent start;
    XEvent ev;
    if(!(display = XOpenDisplay(0x0))) return 1;
    screen = DefaultScreen(display);
    root = DefaultRootWindow(display);
    //win_focus = getcolor(FOCUS);
    //win_unfocus = getcolor(UNFOCUS);
    /*XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F1")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(display, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(display, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);*/
    XSync(display, false);
    XGrabServer(display);
    unsigned int num_top_level_windows;
    Window returned_root, returned_parent;
    Window* top_level_windows;
    XQueryTree(
      display,
      root,
      &returned_root,
      &returned_parent,
      &top_level_windows,
      &num_top_level_windows);
    if (root != returned_root){
        fprintf(stderr, "mismatched roots\n");
        exit(1);
    }
    printf("num_top_level_windows : %d\n", num_top_level_windows);
    for (unsigned int i = 0; i < num_top_level_windows; ++i) {
        Frame(top_level_windows[i], true);
    }
    XFree(top_level_windows);
    XUngrabServer(display);
    for (;;){
        XNextEvent(display, &ev);
        if (ev.type == KeyPress && ev.xkey.subwindow != None){
            XRaiseWindow(display, ev.xkey.subwindow);
        /*} else if (ev.type == ButtonPress && ev.xbutton.subwindow != None){
            XGrabPointer(display, ev.xbutton.subwindow, True, PointerMotionMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
            XGetWindowAttributes(display, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        } else if (ev.type == MotionNotify){
            int xdiff, ydiff;
            while(XCheckTypedEvent(display, MotionNotify, &ev));
            xdiff = ev.xbutton.x_root - start.x_root;
            ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(display, ev.xmotion.window,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));*/
        } else if (ev.type == MapRequest){
            //Frame(ev.xmaprequest.window, False);
            mapWindow(ev);
        } /*else if(ev.type == ButtonRelease){
            XUngrabPointer(display, CurrentTime);
        }*/
    }
    return 0;
}
#include <X11/Xlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

Display* display;
Window root;

void mapWindow(XEvent ev){
    const unsigned int BORDER_WIDTH = 3;
    //const unsigned long BORDER_COLOR = 0x000000;
    const unsigned long BORDER_COLOR = 0x0000ff;
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
        BG_COLOR
    );
    //XMapWindow(display, ev.xmaprequest.window);
    XMapWindow(display, frame);
}


int main(){
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;
    if(!(display = XOpenDisplay(0x0))) return 1;
    root = DefaultRootWindow(display);
    XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F1")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(display, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(display, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    for (;;){
        XNextEvent(display, &ev);
        if (ev.type == KeyPress && ev.xkey.subwindow != None){
            XRaiseWindow(display, ev.xkey.subwindow);
        } else if (ev.type == ButtonPress && ev.xbutton.subwindow != None){
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
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        } else if (ev.type == MapRequest){
            //Frame(ev.xmaprequest.window, False);
            mapWindow(ev);
        } else if(ev.type == ButtonRelease){
            XUngrabPointer(display, CurrentTime);
        }
    }
    return 0;
}
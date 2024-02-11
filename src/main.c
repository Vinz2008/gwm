#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "window_list.h"
#include "utils.h"
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define BORDER_WIDTH 3
#define BORDER_COLOR 0xff0000
#define BG_COLOR 0x0000ff

Display* display;
Window root;
Window topBarWindow;
int screen;
struct window_list* w_list;
/*static unsigned int win_focus;
static unsigned int win_unfocus;*/

unsigned long getcolor(const char* color) {
    XColor c;
    Colormap map = DefaultColormap(display,screen);

    if(!XAllocNamedColor(display,map,color,&c,&c)){
        printf("Error parsing color!");
        exit(1);
    }

    return c.pixel;
}

int OnXError(Display* display_with_error, XErrorEvent* e){
    char error_buf[1024];
    XGetErrorText(display_with_error, e->error_code, error_buf, sizeof(error_buf));
    printf("Received X Error : \n\tRequest : %d - %s\n\tError code : %d - %s\n\tResource ID : %ld\n", (int)e->request_code, XRequestCodeToString(e->request_code), (int)e->error_code, error_buf, e->resourceid);
    return 0;
}


void Frame(Window w, bool was_created_before_window_manager){
    if (is_in_window_list(w_list, w)){
        return;
    }
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
    append_window_list(w_list, create_window(w, frame));
    fprintf(stderr, "framed window : %ld with frame %ld\n", w, frame);
}

void UnFrame(Window w){
    if (!is_in_window_list(w_list, w)){
        fprintf(stderr, "calling unframe on a windows that is not found");
        exit(1);
    }

    const Window frame = find_frame_window_list(w_list, w);
    XUnmapWindow(display, frame);
    XReparentWindow(display, w, root, 0, 0);
    XRemoveFromSaveSet(display, w);
    XDestroyWindow(display, frame);
    remove_from_window_list(w_list, w);
    fprintf(stderr, "unframed window : %ld with frame %ld\n", w, frame);
}


// TODO : instead of passing the XEvent, pass the specific event needed like XMapRequest
void mapWindow(XEvent ev){
    Frame(ev.xmaprequest.window, false);
    XMapWindow(display, ev.xmaprequest.window);
}

void onUnmapNotify(XEvent ev){
    if (!is_in_window_list(w_list, ev.xunmap.window)){
        printf("ignoring UnMapNotify for non-client window\n");
    }
    if (ev.xunmap.event == root){
        return;
    }
    UnFrame(ev.xunmap.window);
}

void onConfigureRequest(XEvent ev){
    XWindowChanges changes;
    changes.x = ev.xconfigurerequest.x;
    changes.y = ev.xconfigurerequest.y;
    changes.width = ev.xconfigurerequest.width;
    changes.height = ev.xconfigurerequest.height;
    changes.border_width = ev.xconfigurerequest.border_width;
    changes.sibling = ev.xconfigurerequest.above;
    changes.stack_mode = ev.xconfigurerequest.detail;
    if (is_in_window_list(w_list, ev.xconfigurerequest.window)){
        const Window frame = find_frame_window_list(w_list, ev.xconfigurerequest.window);
        XConfigureWindow(display, frame, ev.xconfigurerequest.value_mask, &changes);
    }
    XConfigureWindow(display, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &changes);

}


void frameAlreadyOpenedWindows(){
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
}

void drawTopBar(){
    int screen = DefaultScreen(display);
    topBarWindow = XCreateSimpleWindow(display, root, 0, 0, DisplayWidth(display, screen), 10, BORDER_WIDTH, BORDER_COLOR, BG_COLOR);
    XSelectInput(display, topBarWindow, SubstructureRedirectMask | SubstructureNotifyMask);
    XMapWindow(display, topBarWindow);
}

int main(){
    //XWindowAttributes attr;
    //XButtonEvent start;
    w_list = create_window_list();
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
    XSetErrorHandler(OnXError);
    //drawTopBar();
    frameAlreadyOpenedWindows();
    for (;;){
        XNextEvent(display, &ev);
        printf("TEST\n");
        if (ev.type == MapRequest){
            //Frame(ev.xmaprequest.window, False);
            mapWindow(ev);
        /*} else if (ev.type == KeyPress && ev.xkey.subwindow != None){
            XRaiseWindow(display, ev.xkey.subwindow);*/
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
        } else if (ev.type == ConfigureRequest){
            onConfigureRequest(ev);
        } else if (ev.type == UnmapNotify){
            onUnmapNotify(ev);
        } else if (ev.type == DestroyNotify){
            printf("destroying window\n");
        }
        
         /*else if(ev.type == ButtonRelease){
            XUngrabPointer(display, CurrentTime);
        }*/
    }
    destroy_window_list(w_list);
    return 0;
}
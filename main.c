#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/signal.h>

#define ERROR(x) fprintf(stderr, x); exit(1)


typedef struct {
    Window* clients;
    size_t used;
    size_t size;
} ClientList;


#define WIDTH   400
#define HEIGHT  200

FILE* logFile;
Bool wm_detected_;

int sw, sh, wx, wy;
unsigned int ww, wh;


Display* display;
XButtonEvent mouse;
Window root;

#define win_size(W, gx, gy, gw, gh) \
    XGetGeometry(display, W, &(Window){0}, gx, gy, gw, gh, \
                 &(unsigned int){0}, &(unsigned int){0})

struct window {
	//xcb_drawable_t id;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
};


struct window* winlist;

ClientList clientList;

int xerror() { 
    return 0;
}

void initClientList(ClientList* clientList, size_t initalSize){
    clientList->clients = malloc(initalSize * sizeof(clientList));
    clientList->used = 0;
    clientList->size = initalSize;
}

void addClientToList(ClientList* clientList, Window client){
    if (clientList->used == clientList->size){
        clientList->size *=2;
        clientList->clients = realloc(clientList->clients, clientList->size * sizeof(Window));
    }
    clientList->clients[clientList->used++] = client;
}

void removeClientToList(ClientList* clientList, Window client){
    ClientList clientList2;
    clientList2.clients = malloc((clientList->used - 1) * sizeof(Window));
    int i2 = 0;
    for (int i = 0; i < clientList->used - 1; i++){
        if (clientList->clients[i2] != client){
            clientList2.clients[i] = clientList->clients[i2];
        }
        i2++;

    }
    clientList = &clientList2;
}

void emptyClientList(ClientList* clientList){
    free(clientList->clients);
    clientList->clients = NULL;
    clientList->used = clientList->size = 0;
}

Bool isClientIsInClientList(ClientList* clientList, Window client){
    for (int i = 0; i < clientList->size; i++){
        if (clientList->clients[i] == client){
            return True;
        }
    }
    return False;
}

int getPosClientInClientList(ClientList* clientList, Window client){
    int pos = -1;
    for (int i = 0; i < clientList->size; i++){
        if (clientList->clients[i] == client){
            pos = i;
        }
    }
    if (pos != -1){
        return pos;
    } else {
        ERROR("pos not found\n");
    }
}
int OnWMDetected(Display* d, XErrorEvent* e){
    if (e->error_code == BadAccess){
        wm_detected_ = True;
    }
    return 0;
}

void Frame(Window w, Bool wasCreatedBeforeWM){ 
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;
    XWindowAttributes x_window_attrs;
    XGetWindowAttributes(display, w, &x_window_attrs);
    if (wasCreatedBeforeWM) {
    if (x_window_attrs.override_redirect ||
        x_window_attrs.map_state != IsViewable) {
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
}

void unFrame(Window w){
    Window frame = clientList.clients[getPosClientInClientList(&clientList, w)];
    XUnmapWindow(display, frame);
    XReparentWindow(
      display,
      w,
      root,
      0, 0);
    XRemoveFromSaveSet(display, w);
    XDestroyWindow(display, frame);
    removeClientToList(&clientList, w);
}

void input_grab(Window root) {
}

void onConfigureRequest(XConfigureRequestEvent* ev){
    XWindowChanges changes;
    changes.x = ev->x;
    changes.y = ev->y;
    changes.width = ev->width;
    changes.height = ev->height;
    changes.sibling = ev->above;
    changes.stack_mode = ev->detail;

    if (isClientIsInClientList(&clientList, ev->window)){
        const Window frame = clientList.clients[getPosClientInClientList(&clientList, ev->window)];
        XConfigureWindow(display, frame, ev->value_mask, &changes);
    }

    XConfigureWindow(display, ev->window, ev->value_mask, &changes);
}

void onMapRequest(XMapRequestEvent* ev){
    Frame(ev->window, False);
    XMapWindow(display, ev->window);
}

void onMotionNotify(XMotionEvent* ev){
    Window frame = clientList.clients[getPosClientInClientList(&clientList, ev->window)];
}

void onCreateNotify(XCreateWindowEvent* ev){}

void onDestroyNotify(XDestroyWindowEvent* ev){}

void onReparentNotify(XReparentEvent* ev){}

void onMapNotify(XMapEvent* ev){}

void onUnmapNotify(XUnmapEvent* ev){
    if (!isClientIsInClientList(&clientList, ev->window)){
        // Ignoring UnmapNotify for non-client window
        return;
    }
    if (ev->event == root){
        // Ignoring UnmapNotify for reparented pre-existing window
        return;
    }
    unFrame(ev->window);
}

void start_window_manager(){
    XEvent e;
	display = XOpenDisplay(0);
	if (display == NULL){
		ERROR("ERROR : can't open display\n");
	}
    signal(SIGCHLD, SIG_IGN);
    wm_detected_ = False;
    XSetErrorHandler(&OnWMDetected);
    XSelectInput(
      display,
      root,
      SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display, False);
    if (wm_detected_) {
        printf("Detected another window manager on display %s\n", XDisplayString(display)); exit(1); // for format couldn't put ERROR
    }
    XSetErrorHandler(xerror);
    int screen = DefaultScreen(display);
    root  = RootWindow(display, screen);
    sw = XDisplayWidth(display, screen);
    sh = XDisplayHeight(display, screen);
    XSelectInput(display,  root, SubstructureRedirectMask);
    XDefineCursor(display, root, XCreateFontCursor(display, 68));
    input_grab(root);
    XGrabServer(display);
    Window returned_root, returned_parent;
    Window* top_level_windows;
    unsigned int num_top_level_windows;
    XQueryTree(
      display,
      root,
      &returned_root,
      &returned_parent,
      &top_level_windows,
      &num_top_level_windows);
    for (unsigned int i = 0; i < num_top_level_windows; ++i) {
    Frame(top_level_windows[i], True /* was_created_before_window_manager */);
    }
    XFree(top_level_windows);
    XUngrabServer(display);
    while (1 && !XNextEvent(display, &e)) {
        switch (e.type)
        {
        case ConfigureRequest:
            onConfigureRequest(&e.xconfigurerequest);
            break;
        case MapRequest:
            onMapRequest(&e.xmaprequest);
            break;
        case CreateNotify:
            onCreateNotify(&e.xcreatewindow);
            break;
        case DestroyNotify:
            onDestroyNotify(&e.xdestroywindow);
            break;
        case ReparentNotify:
            onReparentNotify(&e.xreparent);
            break;
        case MapNotify:
            onMapNotify(&e.xmap);
            break;
        case UnmapNotify:
            onUnmapNotify(&e.xunmap);
            break;
        default:
            printf("Event unknown\n");
            break;
        }
    }
    XCloseDisplay(display);
    return;
	
}

int start_program(char* program_path) {
	pid_t pid = fork();
	if (-1 == pid)
    {
        fprintf(logFile, "ERROR : couldn't create a fork");
        return -1;
    }
    else if (0 == pid)
    {
		char *argv[2];
		argv[0] = program_path;
        argv[1] = NULL;
		if (-1 == execvp(program_path, argv)){
            fprintf(logFile,"EROR in execve");
            exit(1);
        }
	}
	return 0;
}



int main() {
    initClientList(&clientList, 1);
	logFile = fopen("log.txt", "w");
	if(logFile == NULL) {
        printf("log file can't be opened\n");
        exit(1);
    }
	start_window_manager();
	
	fclose(logFile);
	return 0;
}


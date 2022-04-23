#include <xkbcommon/xkbcommon.h>
#include <xcb/xcb.h>
#include <unistd.h>


int main() {
	xcb_connection_t *c;
	xcb_screen_t *screen;
	xcb_drawable_t win;
	xcb_gcontext_t foreground;
	uint32_t mask;
	uint32_t values[2];
	struct xkb_context *ctx;
	c = xcb_connect(NULL, NULL); // connect to X server
	screen = xcb_setup_roots_iterator(xcb_get_setup (c)).data; // get first screen

	// create black foreground
	win = screen->root;
	foreground = xcb_generate_id(c); 
	mask = XCB_GC_FOREGROUND | XCB_GC_FOREGROUND;
	values[0] = screen->black_pixel;
	values[1] = 0;
	xcb_create_gc(c, foreground, win, mask, values);

	win = xcb_generate_id(c); // ask window id

	//create window
	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE;
	xcb_create_window(c, // connection to X
					  XCB_COPY_FROM_PARENT, // depth
					  win, // window id
					  screen->root, // parent window
					  0, 0, // x and y of the window
					  150, 150, // width and height
					  10, // border width
					  XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
					  screen->root_visual, // visual
					  mask, values // masks
	);
	xcb_map_window(c, win);
	xcb_flush(c);
	pause();
	return 0;
}

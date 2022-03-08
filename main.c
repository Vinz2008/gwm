#include <xkbcommon/xkbcommon.h>
#include <xcb/xcb.h>
#include <unistd.h>


int main() {
	xcb_connection_t *c;
	xcb_screen_t *screen;
	xcb_window_t win;
	struct xkb_context *ctx;
	c = xcb_connect(NULL, NULL);
	screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;
	win = xcb_generate_id(c);
	xcb_create_window (c,                             /* Connection          */
                     XCB_COPY_FROM_PARENT,          /*depth (same as root)*/
                     win,                           /*window Id*/
                     screen->root,                  /*parent window*/
                     0, 0,                          /*x, y*/
                     150, 150,                      /* width, height*/
                     10,                            /*border_width*/
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, /*class */
                     screen->root_visual, /*visual*/
                     0, NULL);
	xcb_map_window (c, win);
	sleep(1);
	return 0;
}

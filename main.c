#include <xkbcommon/xkbcommon.h>
#include <xcb/xcb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int main() {
	xcb_connection_t *c;
	xcb_screen_t *screen;
	xcb_drawable_t win;
	xcb_gcontext_t foreground;
	xcb_generic_event_t *e;
	uint32_t mask;
	uint32_t values[2];
	xcb_point_t points[] = {
		{10, 10}, {10, 20}, {20, 10}, {20, 20}
	};
	xcb_point_t polyline[] = {
		{50, 10}, {5, 20}, {25, -20}, {20, 10}
	};
	xcb_segment_t segments[] = {
		{100, 10, 140, 30}, {110, 25, 130, 60}
	};
	xcb_rectangle_t rectangles[] = {
		{10, 50, 40, 20}, {80, 50, 10, 40}
	};
	xcb_arc_t arcs[] = {
		{10, 100, 60, 40, 0, 90 << 6},
		{90, 100, 55, 40, 0, 270 << 6}
	};
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
	while ((e = xcb_wait_for_event(c))) {
		switch (e->response_type & ~0x80)
		{
		case XCB_EXPOSE:
			xcb_poly_point(c, XCB_COORD_MODE_ORIGIN, win, foreground, 4, points);
			xcb_poly_line(c, XCB_COORD_MODE_PREVIOUS, win, foreground, 4, polyline);
			xcb_poly_segment(c, win, foreground, 2, segments);
			xcb_poly_rectangle(c, win, foreground, 2, rectangles);
			xcb_poly_arc(c, win, foreground, 2, arcs);
			xcb_flush(c);
			break;
		
		default:
			// unknown event that is ignored
			break;
		}
	}
	free(e);
	sleep(5);
	return 0;
}

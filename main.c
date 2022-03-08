#include <xcb/xcb.h>

int main() {
	xcb_connection_t *c;
	xcb_screen_t *screen;
	c = xcb_connect(NULL, NULL);
	return 0;
}

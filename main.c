#include <xkbcommon/xkbcommon.h>
#include <xcb/xcb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_modifiers (uint32_t mask)
{
  const char **mod, *mods[] = {
    "Shift", "Lock", "Ctrl", "Alt",
    "Mod2", "Mod3", "Mod4", "Mod5",
    "Button1", "Button2", "Button3", "Button4", "Button5"
  };
  printf ("Modifier mask: ");
  for (mod = mods ; mask; mask >>= 1, mod++)
    if (mask & 1)
      printf(*mod);
  putchar ('\n');
}


int main() {
	xcb_connection_t *c;
	xcb_screen_t *screen;
	xcb_drawable_t win;
	xcb_gcontext_t foreground;
	xcb_generic_event_t *e;
	uint32_t mask;
	uint32_t values[2];
	char string[] = "gwm";
	xcb_rectangle_t rectangles[] = {
    	{40, 40, 20, 20},
  	};
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
			xcb_expose_event_t *ev = (xcb_expose_event_t *)e;
			xcb_poly_rectangle(c, win, foreground, 1, rectangles);
			xcb_image_text_8 (c, strlen(string), win, foreground, 20, 20, string);
			xcb_flush(c);
			break;
		case XCB_BUTTON_PRESS: {
      		xcb_button_press_event_t *ev = (xcb_button_press_event_t *)e;
      		print_modifiers(ev->state);

      		switch (ev->detail) {
      			case 4:
				  	//printf ("Wheel Button up in window %ld, at coordinates (%d,%d)\n", ev->event, ev->event_x, ev->event_y);
        			break;
      			case 5:
        			//printf ("Wheel Button down in window %ld, at coordinates (%d,%d)\n",ev->event, ev->event_x, ev->event_y);
        			break;
          return 0;
      			default:
        		//printf ("Button %d pressed in window %ld, at coordinates (%d,%d)\n",ev->detail, ev->event, ev->event_x, ev->event_y);
      		}
      		break;
  		}
    	case XCB_BUTTON_RELEASE: {
      		xcb_button_release_event_t *ev = (xcb_button_release_event_t *)e;
      		print_modifiers(ev->state);
      		//printf ("Button %d released in window %ld, at coordinates (%d,%d)\n",ev->detail, ev->event, ev->event_x, ev->event_y);
			putchar(ev->detail);
      		break;
    	}
    	case XCB_MOTION_NOTIFY: {
      		xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)e;
      		//printf ("Mouse moved in window %ld, at coordinates (%d,%d)\n",ev->event, ev->event_x, ev->event_y);
      		break;
    	}
    	case XCB_ENTER_NOTIFY: {
      		xcb_enter_notify_event_t *ev = (xcb_enter_notify_event_t *)e;
      		//printf ("Mouse entered window %ld, at coordinates (%d,%d)\n",ev->event, ev->event_x, ev->event_y);
      		break;
    	}
    	case XCB_LEAVE_NOTIFY: {
      		xcb_leave_notify_event_t *ev = (xcb_leave_notify_event_t *)e;
      		//printf ("Mouse left window %ld, at coordinates (%d,%d)\n",ev->event, ev->event_x, ev->event_y);
      		break;
		}
    	case XCB_KEY_PRESS: {
      		xcb_key_press_event_t *ev = (xcb_key_press_event_t *)e;
      		print_modifiers(ev->state);
      		//printf ("Key pressed in window %ld\n",ev->event);
      		break;
    	}
    	case XCB_KEY_RELEASE: {
      		xcb_key_release_event_t *ev = (xcb_key_release_event_t *)e;
      		print_modifiers(ev->state);
       		switch (ev->detail) {
        		case 9:/* ESC */
          			free (e);
          			xcb_disconnect (c);
          			exit(0);
        	}
      		//printf ("Key released in window %ld\n",ev->event);
      		break;
    	}
    	default:
      		printf("Unknown event: %d\n", e->response_type);
      		break;
    }
		free(e);
	}
	endloop:
	return 0;
}

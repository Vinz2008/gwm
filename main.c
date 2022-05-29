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
	xcb_screen_iterator_t screen_iter;
	xcb_connection_t *c;
	xcb_screen_t *screen;
	const xcb_setup_t *setup;
	int screen_number;
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
	if (!c) {
    printf("ERROR: can't connect to an X server\n");
    return -1;
  	}


	  // get first screen
	setup = xcb_get_setup(c);
  	screen = NULL;
  	screen_iter = xcb_setup_roots_iterator(setup);
 	//for (; screen_iter.rem != 0; --screen_number, xcb_screen_next(&screen_iter))
    //if (screen_number == 0)
    //{
    screen = screen_iter.data;
    //break;
    //}
	if (!screen) {
    printf("ERROR: can't get the current screen\n");
    xcb_disconnect(c);
    return -1;
	}

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
	while (1) {
		e = xcb_poll_for_event(c);
		if (e){
		switch (e->response_type & ~0x80)
		{
		case XCB_EXPOSE:
			xcb_expose_event_t *ev = (xcb_expose_event_t *)e;
			xcb_poly_rectangle(c, win, foreground, 1, rectangles);
			xcb_image_text_8(c, strlen(string), win, foreground, 20, 20, string);
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
          			free(e);
          			xcb_disconnect(c);
          			exit(0);
					return 0;
        	}
      		//printf ("Key released in window %ld\n",ev->event);
      		break;
    	}
		case XCB_CONFIGURE_REQUEST: {
			xcb_configure_request_event_t *ev = (xcb_configure_request_event_t*)e;
			const uint32_t changes[] = {
				ev->x,
				ev->y,
				ev->width,
				ev->height
			};
			xcb_configure_window(c, ev->window, ev->value_mask, changes);
			break;
		}
		case XCB_MAP_REQUEST: {
			xcb_map_request_event_t *ev = (xcb_map_request_event_t*)e;
			const unsigned int BORDER_WIDTH = 3;
			const unsigned long BORDER_COLOR = 0xff0000;
			const unsigned long BG_COLOR = 0x0000ff;
			uint32_t values_temp[] = { screen->white_pixel, XCB_EVENT_MASK_EXPOSURE };
			xcb_get_geometry_reply_t *geometry_reply = xcb_get_geometry_reply(c, xcb_get_geometry (c, ev->window), NULL);
			xcb_get_window_attributes_reply_t *window_attributes = xcb_get_window_attributes_reply(c, xcb_get_window_attributes(c, ev->window), NULL);
			uint32_t temp_id = xcb_generate_id(c);
			xcb_create_window(
				c,
				geometry_reply->depth,
				temp_id,
				screen->root,
				geometry_reply->x,
				geometry_reply->y,
				geometry_reply->width,
				geometry_reply->height,
				geometry_reply->border_width,
				window_attributes->_class,
				window_attributes->visual,
				window_attributes->all_event_masks,
				values_temp
			);
			xcb_map_window(c, temp_id);
			const static uint32_t values[] = { 10, 20 };
			xcb_configure_window (c, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
			free(geometry_reply);
			break;
		}
    	default:
      		printf("Unknown event: %d\n", e->response_type);
      		break;
    }
	}
	free(e);
	}
	return 0;
}


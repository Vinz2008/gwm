#include <xcb/xcb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* logFile;
xcb_connection_t *c;
xcb_screen_t *screen;

struct window {
	xcb_drawable_t id;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
};


struct window* winlist;

uint32_t getcolor(const char *colstr)
{
    xcb_alloc_named_color_reply_t *col_reply;
    xcb_colormap_t colormap;
    xcb_generic_error_t *error;
    xcb_alloc_named_color_cookie_t colcookie;

    colormap = screen->default_colormap;
    colcookie = xcb_alloc_named_color(c, colormap, strlen(colstr), colstr);
    col_reply = xcb_alloc_named_color_reply(c, colcookie, &error);
    if (NULL != error)
    {
        printf("ERROR mcwm: Couldn't get pixel value for colour %s. "
                "Exiting.\n", colstr);

        xcb_disconnect(c);
        exit(1);
    }

    return col_reply->pixel;
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
		if (-1 == execvp(program_path, argv))
        {
            fprintf(logFile,"EROR in execve");
            exit(1);
        }

	}
	return 0;
}


void resize_window(xcb_drawable_t window, uint16_t width, uint16_t height){
	uint32_t values[2];
    if (window == screen->root|| window == 0){
        return;
    }
	values[0] = width;
    values[1] = height;
	 xcb_configure_window(c, window,
                         XCB_CONFIG_WINDOW_WIDTH
                         | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(c);
}

void move_window(xcb_drawable_t window, uint16_t x, uint16_t y){
	uint32_t values[2];
	if (window == screen->root|| window == 0){
		return;
	}
	values[0] = x;
    values[1] = y;
	xcb_configure_window(c, window, XCB_CONFIG_WINDOW_X
                         | XCB_CONFIG_WINDOW_Y, values);
    xcb_flush(c);
}

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
	logFile = fopen("log.txt", "w");
	if(logFile == NULL) {
        printf("log file can't be opened\n");
        exit(1);
    }
	xcb_screen_iterator_t screen_iter;
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
    fprintf(logFile,"ERROR: can't connect to an X server\n");
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
    fprintf(logFile,"ERROR: can't get the current screen\n");
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
					  screen->width_in_pixels/2, 0, // x and y of the window
					  screen->width_in_pixels/2, screen->height_in_pixels, // width and height
					  3, // border width
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
      		//printf("Button %d released in window %ld, at coordinates (%d,%d)\n",ev->detail, ev->event, ev->event_x, ev->event_y);
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
      		fprintf(logFile,"Key pressed in window %d\n",ev->detail);
      		break;
    	}
    	case XCB_KEY_RELEASE: {
      		xcb_key_release_event_t *ev = (xcb_key_release_event_t *)e;
      		print_modifiers(ev->state);
       		switch (ev->detail) {
        		case 9:/* ESC */
          			free(e);
          			xcb_disconnect(c);
					return 0;
        	}
      		fprintf(logFile, "Key released in window %d\n",ev->detail);
      		break;
    	}
		case XCB_CONFIGURE_REQUEST: {
			xcb_configure_request_event_t *ev = (xcb_configure_request_event_t*)e;
			const uint32_t changes[] = {
				/*ev->x*/ 0,
				/*ev->y*/0,
				/*ev->width*/screen->width_in_pixels/2,
				/*ev->height*/ screen->height_in_pixels
			};
			xcb_configure_window(c, ev->window, ev->value_mask, changes);
			fprintf(logFile, "configured window %d in x:%d and y: %d with a height %d of and a width of %d", ev->window, ev->x, ev->y, ev->height, ev->width);
			break;
		}
		case XCB_MAP_REQUEST: {
			xcb_map_request_event_t *ev = (xcb_map_request_event_t*)e;
			/*const unsigned int BORDER_WIDTH = 3;
			const unsigned long BORDER_COLOR = 0xff0000;
			const unsigned long BG_COLOR = 0x0000ff;
			uint32_t values_temp2[2];
			uint32_t mask_temp = 0;
			uint32_t values_temp[] = { screen->white_pixel, XCB_EVENT_MASK_EXPOSURE };
			xcb_get_geometry_reply_t *geometry_reply = xcb_get_geometry_reply(c, xcb_get_geometry (c, ev->window), NULL);
			xcb_get_window_attributes_reply_t *window_attributes = xcb_get_window_attributes_reply(c, xcb_get_window_attributes(c, ev->window), NULL);*/
			/*uint32_t temp_id = xcb_generate_id(c);
			xcb_create_window(
				c,
				geometry_reply->depth,
				temp_id,
				geometry_reply->root,
				geometry_reply->x,
				geometry_reply->y,
				geometry_reply->width,
				geometry_reply->height,
				geometry_reply->border_width,
				window_attributes->_class,
				window_attributes->visual,
				window_attributes->all_event_masks,
				values_temp
			);*/
			//values_temp2[0] = getcolor("grey40");
			/*values_temp2[0] = BORDER_COLOR;
			xcb_change_window_attributes(c, ev->window, XCB_CW_BORDER_PIXEL, values_temp2);
			mask_temp = XCB_CW_EVENT_MASK;
    		values_temp2[0] = XCB_EVENT_MASK_ENTER_WINDOW;
    		xcb_change_window_attributes_checked(c, ev->window, mask, values_temp2);
			xcb_change_save_set(c, XCB_SET_MODE_INSERT, ev->window);
    		xcb_flush(c);
			move_window(ev->window, 0, 0);
			resize_window(ev->window, screen->width_in_pixels/2, screen->height_in_pixels);*/
			xcb_map_window(c, ev->window);
			uint32_t vals[5];
			int window_width = 600;
			int window_height = 400;
			vals[0] = (screen->width_in_pixels / 2) - (window_width / 2);
			vals[1] = (screen->height_in_pixels / 2) - (window_height / 2);
			vals[2] = window_width;
			vals[3] = window_height;
			vals[4] = 1; // border width
			xcb_configure_window(c, ev->window, XCB_CONFIG_WINDOW_X |XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
			//fprintf(logFile, "window %d configured at x: 0 and y: 0", ev->window);
			/*const static uint32_t values[] = { 10, 20 };
			xcb_configure_window (c, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);*/
			//free(geometry_reply);
			xcb_flush(c);
			values[0] = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE;
			xcb_change_window_attributes_checked(c, ev->window,XCB_CW_EVENT_MASK, values);
			break;
		}
    	default:
      		fprintf(logFile,  "Unknown event: %d\n", e->response_type);
      		break;
    }
	}
	free(e);
	}
	fclose(logFile);
	return 0;
}


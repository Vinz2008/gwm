#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct {
    Window window;
    Window frame;
} window_t;

struct window_list {
    window_t* list;
    size_t capacity;
    size_t used;
};

struct window_list* create_window_list();
void destroy_window_list(struct window_list* list);

bool is_in_window_list(struct window_list* list, Window window);

void append_window_list(struct window_list* list, window_t window);
window_t create_window(Window w, Window frame);

Window find_frame_window_list(struct window_list* list, Window window);
void remove_from_window_list(struct window_list* list, Window window);
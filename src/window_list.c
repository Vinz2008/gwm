#include "window_list.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

window_t create_window(Window w, Window frame){
    return (window_t){w, frame};
}

struct window_list* create_window_list(){
    struct window_list* w_list = malloc(sizeof(struct window_list));
    w_list->used = 0;
    w_list->capacity = 5;
    w_list->list = malloc(sizeof(window_t)*w_list->capacity);
    return w_list;
}

void append_window_list(struct window_list* list, window_t window){
    if (list->used == list->capacity){
        list->capacity += 3;
        list->list = realloc(list->list, sizeof(window_t)*list->capacity);
    }
    list->list[list->used++] = window;
}

bool is_in_window_list(struct window_list* list, Window window){
    for (int i = 0; i < list->used; i++){
        if (list->list[i].window == window){
            return true;
        }
    }
    return false;
}

Window find_frame_window_list(struct window_list* list, Window window){
    for (int i = 0; i < list->used; i++){
        if (list->list[i].window == window){
            return list->list[i].frame;
        }
    }
    fprintf(stderr, "searching the frame of a window that is not found\n");
    exit(1);
}

void remove_from_window_list(struct window_list* list, Window window){
    int pos = -1;
    for (int i = 0; i < list->used; i++){
        if (list->list[i].window == window){
            pos = i;
            break;
        }
    }
    if (pos == -1){
        fprintf(stderr, "removing the frame of a window that is not found\n");
        exit(1);
    }
    window_t* window_list_buf = list->list;
    list->capacity -= 3;
    list->list = malloc(sizeof(window_t)*list->capacity);
    memcpy(list->list, window_list_buf, sizeof(window_t)*pos);
    memcpy(list->list+pos+1, window_list_buf+pos+2, sizeof(window_t)*pos);
    list->used -= 1;
}


void destroy_window_list(struct window_list* list){
    free(list->list);
    free(list);
}
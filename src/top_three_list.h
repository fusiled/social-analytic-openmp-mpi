#ifndef TOP_THREE_LIST
#define TOP_THREE_LIST

#include "top_three.h"

typedef struct top_three_list_element
{
    top_three * element;
    struct top_three_list_element* next;
} top_three_list_element;

typedef struct top_three_list
{
    top_three_list_element* head;
    top_three_list_element* tail;
    int size;
} top_three_list;

// Create an empty list
top_three_list* create_top_three_list();

void add_top_three_element(top_three_list* list, top_three* el);

// CLear the list, but not the top_three elements.
void clear_top_three_list(top_three_list* list);

// Convert into array
top_three** get_top_three_array(top_three_list* list, int* size);




#endif

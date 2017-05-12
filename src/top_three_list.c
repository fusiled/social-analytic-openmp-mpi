#include "top_three.h"
#include "top_three_list.h"

#include "debug_utils.h"

#include <stdlib.h>

top_three_list_element* create_element(top_three* tt);

// Create an empty list
top_three_list* create_top_three_list()
{
    top_three_list* list = malloc(sizeof(top_three_list));
    if (list == NULL)
    {
        return list;
    }
    list->head=NULL;
    list->tail=NULL;
    list->size=0;
    return list;
}

void add_top_three_element(top_three_list* list, top_three* el)
{
    top_three_list_element* new_el = create_element(el);
    if (new_el == NULL)
    {
        print_error("Cannot allocate a top_three_list_element!");
        return;
    }
    // Change the pointer of the last element of the list, taking into account
    // case in which the list is empty
    if (list->size == 0)
    {
        // Add simply the element to the list
        list->head = new_el;
        list->tail = new_el;
        list->size = 1;
    }
    else
    {
        // Change the pointer of the last element
        (list->tail)->next = new_el;
        // Change the tail of the list
        list->tail = new_el;
        list->size = list->size + 1;
    }
}

// CLear the list, but not the top_three elements inside.
void clear_top_three_list(top_three_list* list)
{
    while(list->head)
    {
        top_three_list_element* tmp = (list->head)->next;
        free(list->head);
        list->head=tmp;
    }
    free(list);
}

top_three** get_top_three_array(top_three_list* list, int* size)
{
    top_three** tt_array = malloc(sizeof(top_three*)*(list->size));
    if (tt_array == NULL)
    {
        print_error("Cannot allocate the array of the top_three");
        return NULL;
    }
    // Fill the array
    int i=0;
    top_three_list_element* p = list->head;
    while(p)
    {
        tt_array[i] = p->element;
        i++;
        p = p->next;
    }
    // Save the output size
    *size = (list->size);
    return tt_array;
}

top_three_list_element* create_element(top_three* tt)
{
    top_three_list_element* el = malloc(sizeof(top_three_list_element));
    if (el==NULL)
    {
        return el;
    }
    el->element=tt;
    el->next=NULL;
    return el;
}

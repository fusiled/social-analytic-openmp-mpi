#include "event_list.h"
#include "debug_utils.h"
#include "quicksort.h"

#include <stdlib.h>

valued_event_list_element* create_element(int post_ts, long post_id, int score, valued_event_list_element* next);

void destroy_element(valued_event_list_element* el);

event_list* create_event_list()
{
    event_list* list = malloc(sizeof(event_list));
    if (list == NULL)
    {
        return list;
    }
    list->head=NULL;
    list->size=0;
    return list;
}

void add_element(event_list* list, int post_ts, long post_id, int score)
{
    valued_event_list_element* el = create_element(post_ts, post_id, score, list->head);
    if (el==NULL)
    {
        print_error("Cannot allocate a list element");
        return;
    }
    list->size=list->size+1;
    list->head=el;
}

void clear_valued_event_list(event_list* list)
{
    while(list->head)
    {
        valued_event_list_element* tmp = (list->head)->next;
        free(list->head);
        list->head=tmp;
    }
    free(list);
}

valued_event** get_sorted_array(event_list* list)
{
    // Allocate the memory for the array
    valued_event** ve_array = malloc(sizeof(valued_event*)*(list->size) );
    if (ve_array==NULL)
    {
        print_error("Cannot allocate the array of pointers in get_sorted_array");
        return NULL;
    }
    // Fill the array
    int i=0;
    valued_event_list_element* p = list->head;
    while(p)
    {
        ve_array[i] = p->v;
        i++;
        p = p->next;
    }
    // Sort the array
    sort_valued_events(ve_array,0,(list->size)-1);

    return ve_array;
}

void clear_valued_event(valued_event* ve)
{
    free(ve);
}

valued_event_list_element* create_element(int post_ts, long post_id, int score, valued_event_list_element* next)
{
    valued_event_list_element* el = malloc(sizeof(valued_event_list_element));
    if (el==NULL)
    {
        return NULL;
    }
    valued_event* vv = malloc(sizeof(valued_event));
    if (vv==NULL)
    {
        free(el);
        return NULL;
    }
    vv->post_ts=post_ts;
    vv->post_id=post_id;
    vv->score=score;
    el->v=vv;
    if (next==NULL)
    {
        el->next=NULL;
    }
    else
    {
        el->next=next;
    }

    return el;
}

// It does not delete the valued_event contained into the element of the list
void destroy_element(valued_event_list_element* el)
{
    free(el);
}


void print_valued_event(valued_event * ve)
{
    print_msg("VALUED_EL","post_ts: %d, post_id: %ld, score: %d", ve->post_ts,ve->post_id, ve->score);
}

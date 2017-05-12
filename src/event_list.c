#include "event_list.h"
#include "debug_utils.h"
#include "quicksort.h"

#include <stdlib.h>

valued_event_list_element* create_valued_list_element(int post_ts, long post_id,long user_id, int score, valued_event_list_element* next);

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

void clear_event_list(event_list * list)
{
    valued_event_list_element * tmp = list->head;
    while(tmp!=NULL)
    {  
        valued_event_list_element * tmp2 = tmp->next;
        free(tmp);
        tmp=tmp2;
    }
    free(list);
}

void add_element(event_list* list, int post_ts, long post_id, long user_id, int score)
{
    valued_event_list_element* el = create_valued_list_element(post_ts, post_id, user_id, score, list->head);
    if (el==NULL)
    {
        print_error("Cannot allocate a list element");
        return;
    }
    list->size=list->size+1;
    list->head=el;
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

valued_event_list_element* create_valued_list_element(int post_ts, long post_id, long user_id, int score, valued_event_list_element* next)
{
    valued_event_list_element* el = malloc(sizeof(valued_event_list_element));
    if (el==NULL)
    {
        print_error("cannot malloc a valued_event_list_element");
        return NULL;
    }
    valued_event* vv = malloc(sizeof(valued_event));
    if (vv==NULL)
    {
        print_error("cannoc malloc a valued_event");
        free(el);
        return NULL;
    }
    vv->post_ts=post_ts;
    vv->post_id=post_id;
    vv->user_id=user_id;
    vv->score=score;
    el->v=vv;
    el->next=next;
    return el;
}

// It does not delete the valued_event contained into the element of the list
void destroy_element(valued_event_list_element* el)
{
    free(el);
}


void print_valued_event(valued_event * ve)
{
    print_msg("VALUED_EL","adr: %p, post_ts: %d, post_id: %ld, score: %d",ve, ve->post_ts,ve->post_id, ve->score);
}

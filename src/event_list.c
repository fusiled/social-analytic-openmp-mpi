#include "event_list.h"
#include "debug_utils.h"
//include for sorting facilities
#include "quicksort.h"


#include <stdlib.h>

#include <limits.h>


//PRIVATE function.
//malloc a valued_list_element and initialize it with the specified fields
valued_event_list_element* create_valued_list_element(int post_ts, long post_id,
        long user_id, int score, int n_commenters, valued_event_list_element* next);


//PRIVATE function
//It does not delete the valued_event contained into the element of the list
void destroy_valued_list_element(valued_event_list_element* el);



//PRIVATE FUNCTION. associated to merge_valued_event_with_ref_[something].
//check if it's necessary to quit from the merging loop
char check_if_quit(int * ve_dim,int * counter_ar, int ve_size);


event_list* create_event_list()
{
    //trivial malloc routine
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
    //trivial list exploration
    valued_event_list_element * tmp = list->head;
    while(tmp!=NULL)
    {  
        valued_event_list_element * tmp2 = tmp->next;
        free(tmp);
        tmp=tmp2;
    }
    free(list);
}

void add_element(event_list* list, int post_ts, long post_id, long user_id, int score, int n_commenters)
{
    //trivial list addition
    valued_event_list_element* el = create_valued_list_element(post_ts, post_id, user_id, score, n_commenters,list->head);
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

valued_event_list_element* create_valued_list_element(int post_ts, long post_id, long user_id, int score, int n_commenters, valued_event_list_element* next)
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
    vv->n_commenters=n_commenters;
    vv->score=score;
    el->v=vv;
    el->next=next;
    return el;
}

void destroy_valued_list_element(valued_event_list_element* el)
{
    free(el);
}


void print_valued_event(valued_event * ve)
{
    print_msg("VALUED_EL","adr: %p, post_ts: %d, post_id: %ld, score: %d",ve, ve->post_ts,ve->post_id, ve->score);
}


//PRIVATE FUNCTION. associated to merge_valued_event_with_ref_[something]
char check_if_quit(int * ve_dim,int * counter_ar, int ve_size)
{
    char quit=1;
    for(int i=0; i<ve_size; i++ )
    {
        if(counter_ar[i]<ve_dim[i])
        {
            quit=0;
        }
    }
    return quit;
}

valued_event * merge_valued_event_array_with_ref(valued_event *** ve_arr, int * ve_dim, int ve_size, int * out_size_ref)
{
    print_fine("in merge_valued_event_array_with_ref");
    //malloc space
    int out_size=0;
    for(int i=0; i<ve_size;i++)
    {
        out_size = out_size + ve_dim[i];
    }
    valued_event * out_ref = malloc(sizeof(valued_event)*out_size);
    if(out_ref==NULL)
    {
        print_error("cannot malloc out_ref in merge_valued_event_array_with_ref");
        return NULL;
    }
    int * counter_ar = calloc(sizeof(int),ve_size);
    int out_counter=0;
    while(check_if_quit(ve_dim,counter_ar,ve_size)==0 && out_counter < out_size)
    {
        int index = -1;
        //get element with minimum timestamp
        int ts=INT_MAX;
        for(int i=0; i<ve_size;i++)
        {
            int counter = counter_ar[i];
            if( counter < ve_dim[i] && ts > ve_arr[i][counter]->post_ts )
            {
                ts = ve_arr[i][counter]->post_ts;
                index = i;
            }
        }
        //save selected element
        out_ref[out_counter]=*(ve_arr[index][ counter_ar[index] ]);
        counter_ar[index] = counter_ar[index]+1;
        out_counter++;
    }
    free(counter_ar);
    *out_size_ref=out_size;
    return out_ref;
}


valued_event * merge_valued_event_array(valued_event ** ve_arr, int * ve_dim, int ve_size, int * out_size_ref)
{
     //malloc space
    int out_size=0;
    for(int i=0; i<ve_size;i++)
    {
        out_size = out_size + ve_dim[i];
    }
    valued_event * out_ref = malloc(sizeof(valued_event)*out_size);
    if(out_ref==NULL)
    {
        print_error("cannot malloc out_ref in merge_valued_event_array");
        return NULL;
    }
    int * counter_ar = calloc(sizeof(int),ve_size);
    int out_counter=0;
    while(check_if_quit(ve_dim,counter_ar,ve_size)==0 && out_counter < out_size)
    {
        int index = -1;
        //get element with minimum timestamp
        int ts=INT_MAX;
        for(int i=0; i<ve_size;i++)
        {
            int counter = counter_ar[i];
            if( counter < ve_dim[i] &&  ts > ve_arr[i][counter].post_ts )
            {
                ts = ve_arr[i][counter].post_ts;
                index = i;
            }
        }
        //save selected element
        out_ref[out_counter]=ve_arr[index][ counter_ar[index] ];
        counter_ar[index] = counter_ar[index]+1;
        out_counter++;
    }
    free(counter_ar);
    *out_size_ref=out_size;
    return out_ref;
}
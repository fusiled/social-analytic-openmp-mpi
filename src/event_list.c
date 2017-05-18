#include "event_list.h"
#include "debug_utils.h"
//include for sorting facilities
#include "quicksort.h"
#include "top_three.h"

#include <stdlib.h>

#include <limits.h>


//PRIVATE function.
//malloc a valued_list_element and initialize it with the specified fields
valued_event_list_element* create_valued_list_element(int post_ts, long post_id,
        long user_id, int score, int n_commenters, valued_event_list_element* next);

// PRIVATE function.
// Create a valued list element from an already existent valued event
valued_event_list_element* create_valued_list_element_alreadyExistent(valued_event* ve,
        valued_event_list_element* next);


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




valued_event * new_valued_event(int post_ts, long post_id, long user_id, int score, int n_commenters)
{
    valued_event * ret_ref = malloc(sizeof(valued_event));
    if(ret_ref==NULL)
    {
        print_error("Cannot malloc valued_event");
    }
    ret_ref->post_ts=post_ts;
    ret_ref->post_id=post_id;
    ret_ref->user_id=user_id;
    ret_ref->score=score;
    ret_ref->n_commenters = n_commenters;
    return ret_ref;

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

int add_valued_event_in_order(event_list* list, valued_event* ve)
{
    int change = 0, old_pos = -1, new_pos, list_index=0, found=0;
    valued_event_list_element * current_ve = list->head;
    valued_event_list_element * previous_ve=NULL;
    // First check if the element is into the list.
    while (!found && current_ve)
    {
        if (ve->post_id == current_ve->v->post_id)
        {
            // Remove the post
            old_pos = list_index; // Save the old position
            found = 1; // Notify we have found the post
            list->size=list->size-1; // We remove an element
            if (previous_ve != NULL)
            {
                previous_ve->next=current_ve->next;
            }
            else
            {
                // The post was in the first position
                list->head = current_ve->next;
            }
            // Delete the element from memory
            free(current_ve);
        }
        // Increase the position counter
        list_index++;
        // Update the pointers, if found==0
        if (!found)
        {
            previous_ve = current_ve;
            current_ve = current_ve->next;
        }
    }


    // Now, add the element in the new position
    current_ve = list->head;
    previous_ve=NULL;
    list_index=0;
    int inserted=0;
    while(!inserted && current_ve)
    {
        if (ve->score > current_ve->v->score)
        {
            new_pos=list_index;// Save the new position
            inserted = 1; // Notify we have inserted the post
            list->size=list->size+1; // We add an element
            // Add the element
            valued_event_list_element* el = create_valued_list_element_alreadyExistent(ve,current_ve);
            if (el==NULL)
            {
                print_error("Cannot allocate a list element");
                return -1;
            }
            // Update the pointer of the previous element
            if (previous_ve != NULL)
            {
                previous_ve->next = el;
            }
            else
            {
                // The post is inserted in the first position
                list->head=el;
            }

        }
        // Increase the position counter
        list_index++;
        // Update the pointers
        previous_ve = current_ve;
        current_ve = current_ve->next;
    }
    // Check if there was a change in the top-3
    // First in case of incomplete top-3 there is a change iff old_pos != new_pos. In fact there is a change iff there is a new post.
    change = (list->size < TOP_NUMBER-1) && (old_pos != new_pos);
    // Second, the top-3 has changed
    change = change || (new_pos != old_pos && old_pos >= 0 && old_pos < TOP_NUMBER);
    return change;
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
    valued_event* vv = new_valued_event(post_ts,post_id,user_id,score,n_commenters);
    el->v=vv;
    el->next=next;
    return el;
}

valued_event_list_element* create_valued_list_element_alreadyExistent(valued_event* ve,
        valued_event_list_element* next)
{
    valued_event_list_element* el = malloc(sizeof(valued_event_list_element));
    if(el ==NULL)
    {
        print_error("cannot malloc a valued_event_list_element");
        return NULL;
    }
    el->v=ve;
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
    print_fine("final valued_event array size will be of %d", out_size);
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

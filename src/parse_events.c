#include "parse_events.h"

#include "debug_utils.h"
#include "event_list.h"
#include "top_three.h"
#include "top_three_list.h"

#include <stdlib.h>

#define EMPTY_VALUE -1
// Number of saved posts to update the top three when a post score decreases.
// In fact it may happens that the third post exchanges its position with the fourth.
#define NUM_TOP_POSTS 25

// Add an event into the array at pos (the remaining values are right-shifted)
void add_event_to_the_array(valued_event* array, valued_event* el, int pos);
// Check if the event ve changes the top-three. If yes, the top-three is updated
// and 1 is returned. Otherwise, the function returns 0 (false).
int check_change(valued_event** array, valued_event* ve, int * array_size);
// Create the top-three
top_three* create_top_three(valued_event** ve, int ts);

top_three ** parse_events(valued_event* events_array, int size, int* out_size)
{
    print_warning("parsing events");
    print_fine("v_event_size: %d", size);
    // Initially the top three is empty
    valued_event** tt_current_top = calloc(sizeof(valued_event*),NUM_TOP_POSTS);
    int array_size=0;
    if (tt_current_top == NULL)
    {
        print_error("Cannot allocate a top three array to parse the events and generate the history of the top-threes");
        return NULL;
    }
    // Create a list of top_three
    top_three_list* list = create_top_three_list();
    if (list == NULL)
    {
        free(tt_current_top);
        return NULL;
    }
    for (int i=0; i < size; i++)
    {
        // Save the current timestamp
        int ts = events_array[i].post_ts;
        if (check_change(tt_current_top,events_array+i, &array_size))
        {
            // Add the new top_three to the list
            top_three* tt = create_top_three(tt_current_top, ts);
            if (tt==NULL)
            {
                print_error("Cannot allocate a top_three at timestamp %d", ts);
            }
            else
            {
                add_top_three_element(list, tt);
            }
        }
    }
    // Return the array
    print_fine("finished building the list. making it into an array");
    top_three ** output_top_three_ar =get_top_three_array(list, out_size);
    free(tt_current_top);
    clear_top_three_list(list);
    return output_top_three_ar;
}

// Add an event into the array at pos (the remaining values are right-shifted)
void add_event_to_the_array(valued_event* array, valued_event* el, int pos)
{
    // Case 1: pos equal to TOP_NUMBER-1: Simply insert the event
    if (pos == NUM_TOP_POSTS-1)
    {
        array[NUM_TOP_POSTS-1] = *el;
        return;
    }
    // Other case: we have to shift the other elements
    for (int i=NUM_TOP_POSTS-1; i>pos; i--)
    {
        array[i] = array[i-1];
    }
    // Insert the element
    array[pos] = *el;
}

// Check if the event ve changes the top-three. If yes, the top-three is updated
// and 1 is returned. Otherwise, the function returns 0 (false).
int check_change(valued_event** array, valued_event* ve, int * array_size)
{
    if(*array_size==0)
    {
        array[0]=ve;
        *array_size=1;
        return 1;
    }
    int isChanged = 0, old_pos = -1, new_pos;
    // First check if the event's post is already present into the top-X
    for (int i=0; i<NUM_TOP_POSTS; i++)
    {
        //print_fine("array[%d]:", i);
        if (array[i]!=NULL && ve->post_id == array[i]->post_id)
        {
            old_pos = i;
            array[i] = NULL;
            // Left-shift the other elements.
            for (int j=i, flag=1; flag && j<(*array_size)-1;j++)
            {
                if (array[j+1] != NULL)
                {
                    array[j] = array[j+1];
                }
                else
                {
                    // If there is a null, break
                    flag=0;
                }
            }
            // Reset the last element of the array.
            array[NUM_TOP_POSTS-1] = NULL;
        }
    }
    // Now insert the new event.
    for (int i=0, notDone=1; notDone && i<NUM_TOP_POSTS; i++)
    {
        if (array[i]==NULL || ve->score > array[i]->score )
        {
            //print_fine("right shifting");
            new_pos = i;
            // Right-shift the other elements.
            for (int j=(*array_size) - 1; j>i; j--)
            {
                if (array[j-1] != NULL)
                {
                    array[j] = array[j-1];
                }
            }
            // Insert the event
            array[i] = ve;
            // Set the flag
            notDone = 0;
        }
    }
    // Check if there was a change in the top-3
    // First in case of incomplete top-3 there is a change iff old_pos != new_pos. In fact there is a change iff there is a new post.
    change = (*array_size < TOP_NUMBER-1) && (old_pos != new_pos);
    // Second, the top-3 has changed
    change = change || (new_pos != old_pos && old_pos >= 0 && old_pos < TOP_NUMBER);
    if(change)
    {
        *array_size=*array_size+1;
    }
    return change;
}

// Create the top-three
top_three* create_top_three(valued_event** ve, int ts)
{
    // Define the arrays
    long post_id[TOP_NUMBER], user_id[TOP_NUMBER];
    int post_score[TOP_NUMBER], n_commenters[TOP_NUMBER];
    for(int i=0; i<TOP_NUMBER; i++)
    {
        if (!(ve[i]==NULL))
        {
            post_id[i] = ve[i]->post_id;
            post_score[i] = ve[i]->score;
            // Now the user_id and the n_commenters are empty
            user_id[i] = ve[i]->user_id;
            n_commenters[i] = ve[i]->n_commenters;
        }
        else
        {
            post_id[i] = EMPTY_VALUE;
            post_score[i] = EMPTY_VALUE;
            user_id[i] = EMPTY_VALUE;
            n_commenters[i] = EMPTY_VALUE;
        }
    }
    top_three* tt = new_top_three(ts, post_id, user_id, post_score, n_commenters);
    return tt;
}

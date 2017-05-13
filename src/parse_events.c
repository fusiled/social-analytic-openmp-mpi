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
void add_event_to_the_array(valued_event** array, valued_event* el, int pos);
// Check if the event ve changes the top-three. If yes, the top-three is updated
// and 1 is returned. Otherwise, the function returns 0 (false).
int check_change(valued_event** array, valued_event* ve);
// Create the top-three
top_three* create_top_three(valued_event** ve, int ts);

top_three ** parse_events(valued_event** events_array, int size, int* out_size)
{
    print_fine("v_event_size: %d", size);
    // Initially the top three is empty
    valued_event** tt_current_top = malloc(sizeof(valued_event*)*NUM_TOP_POSTS);
    if (tt_current_top == NULL)
    {
        print_error("Cannot allocate a top three array to parse the events and generate the history of the top-threes");
        return NULL;
    }
    // Create a list of top_three
    top_three_list* list = create_top_three_list();
    print_fine("created top three list");
    if (list == NULL)
    {
        print_error("Cannot allocate the list of top three.");
        free(tt_current_top);
        return NULL;
    }
    print_fine("pre for. addr events_array at: %p. ref to events_array %p", &events_array, events_array);
    for (int i=0; i < size; i++)
    {
        // Save the current timestamp
        int ts = events_array[i]->post_ts;
        if (check_change(tt_current_top,events_array[i]))
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
    return get_top_three_array(list, out_size);
}

// Add an event into the array at pos (the remaining values are right-shifted)
void add_event_to_the_array(valued_event** array, valued_event* el, int pos)
{
    // Case 1: pos equal to TOP_NUMBER-1: Simply insert the event
    if (pos == NUM_TOP_POSTS-1)
    {
        array[NUM_TOP_POSTS-1] = el;
        return;
    }
    // Other case: we have to shift the other elements
    for (int i=NUM_TOP_POSTS-1; i>pos; i--)
    {
        if (!(array[i-1]==NULL))
        {
            array[i] = array[i-1];
        }
    }
    // Insert the element
    array[pos] = el;
}

// Check if the event ve changes the top-three. If yes, the top-three is updated
// and 1 is returned. Otherwise, the function returns 0 (false).
int check_change(valued_event** array, valued_event* ve)
{
    /*for (int i=0; i<NUM_TOP_POSTS; i++)
    {
        if (array[i] == NULL)
        {
            // There is a free space. Add the event to the list and return
            add_event_to_the_array(array, ve, i);
            // The change is notified iff i<TOP_NUMBER
            return (i<TOP_NUMBER);
        }
        else
        {
            if (!(ve->post_id == array[i]->post_id) && ve->score > array[i]->score)
            {
                // Small check: if the post is already into the top_three I have to delete it
                for (int j=i+1; j<NUM_TOP_POSTS; j++)
                {
                    if (array[i]->post_id == ve->post_id)
                    {
                        array[i] = NULL;
                    }
                }
                // Add the event
                add_event_to_the_array(array, ve, i);
                // The change is notified iff i<TOP_NUMBER
                return (i<TOP_NUMBER);
            }
            else if (ve->post_id == array[i]->post_id)
            {
                // Update the event score. Check if it is smaller than the following ones.
                if (ve->score < array[i]->score)
                {
                    // Change the event.
                    array[i] = ve;
                    // Now continue with the checks on the subsequent events in order
                    // to find the new top three.
                    int flag = 1, j;
                    for (j=i+1;flag && j<NUM_TOP_POSTS; j++)
                    {
                        if (ve->score < array[j]->score)
                        {
                            // Invert the two events
                            array[j-1] = array[j];
                        }
                        else
                        {
                            // Stop the checks.
                            flag = 0;
                        }
                    }
                    // Save at pos j-1 the new score iff there were some changes in the top-three.
                    if (!flag)
                    {
                        array[j-1] = ve;
                    }
                }
                else
                {
                    array[i] = ve;
                }
                // The change is notified iff i<TOP_NUMBER
                return (i<TOP_NUMBER);
            }
        }
    }
    // No changes case.
    return 0;*/
    int old_pos = -1, new_pos;
    // First check if the event's post is already present into the top-X
    for (int i=0; i<NUM_TOP_POSTS; i++)
    {
        if (array[i]!=NULL && ve!=NULL &&  ve->post_id == array[i]->post_id)
        {
            //print_fine("left shifting");
            old_pos = i;
            array[i] = NULL;
            // Left-shift the other elements.
            for (int j=i, flag=1; flag && j<NUM_TOP_POSTS-1;j++)
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
        if (array[i]==NULL ||  ve->score > array[i]->score )
        {
            //print_fine("right shifting");
            new_pos = i;
            // Right-shift the other elements.
            for (int j=NUM_TOP_POSTS - 1; j>i; j--)
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
    return (old_pos >= 0 && old_pos != new_pos && old_pos < TOP_NUMBER);
}

// Create the top-three
top_three* create_top_three(valued_event** ve, int ts)
{
    // Define the arrays
    long post_id[TOP_NUMBER], user_id[TOP_NUMBER];
    int post_score[TOP_NUMBER], n_commenters[TOP_NUMBER];
    for(int i=0; i<TOP_NUMBER; i++)
    {
        // Now the user_id and the n_commenters are empty
        user_id[i] = ve[i]->user_id;
        n_commenters[i] = ve[i]->n_commenters;
        if (!(ve[i]==NULL))
        {
            post_id[i] = ve[i]->post_id;
            post_score[i] = ve[i]->score;
        }
        else
        {
            post_id[i] = EMPTY_VALUE;
            post_score[i] = EMPTY_VALUE;
        }
    }
    top_three* tt = new_top_three(ts, post_id, user_id, post_score, n_commenters);
    return tt;
}

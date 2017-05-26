#include "process_events.h"

#include "event_list.h"
#include "event_generator.h"
#include "debug_utils.h"

#include <stdlib.h>
#include <limits.h>

int update_score(int score, char action_type, char is_comment);

int update_nCommenters(int nCommenters, long * commenters_ar, long user_id);

valued_event** process_events(post_block** pb, int size, int * v_event_size)
{
    // For each post block its events are generated and added to a list, then a sorted array is generated from the list
    // Create the list
    event_list * e_list = create_event_list();
    // Process one post block at a time
    int num_events,i,j, score=0;
    for(i=0; i<size; i++)
    {
        int nCommenters=0;
        event** ee = generate_events(pb[i], &num_events);
        ee = sort_generated_events(ee, num_events);
        //create n_commenters array to keep trace of the number of commenters
        //print_info("comment_ar_size: %d", pb[i]->comment_ar_size);
        long * n_commenters_ar=NULL;
        int last_comment_ts=INT_MAX;
        if( pb[i]->comment_ar_size > 0)
        {
            n_commenters_ar = malloc( sizeof(long)*(pb[i]->comment_ar_size) ) ;
            if(n_commenters_ar==NULL)
            {
                print_error("cannot malloc n_commenters_ar");
                return NULL;
            }
        }
        for (j=0;j<num_events;j++)
        {
            score = update_score(score, ee[j]->type, ee[j]->is_comment);
            //print_event(ee[j]);
            if (ee[j]->is_comment && ee[j]->type == CREATION )
            {
                last_comment_ts=ee[j]->timestamp;
                nCommenters = update_nCommenters(nCommenters,n_commenters_ar,ee[j]->user_id);
            }
            if (score <= 0)
            {
                break;
            }
            add_element(e_list,ee[j]->timestamp,ee[j]->post_ts, ee[j]->post_id, pb[i]->user_id , score, nCommenters,last_comment_ts);
        }
        free(n_commenters_ar);
        clear_events(ee,num_events);
    }
    // Return the array after deleting the list
    valued_event** ve = get_sorted_array(e_list);
    *v_event_size = e_list->size; 
    clear_event_list(e_list);
    return ve;
}

// When the score reaches 0 (or less) should be handled outside this function.
int update_score(int score, char action_type, char is_comment)
{
    if (action_type==CREATION && !is_comment)
    {
        // Creation of the post
        score = score + POST_CREATION;
    }
    else if (action_type==CREATION && is_comment)
    {
        // Creation of a comment
        score = score + COMMENT;
    }
    else if (action_type==DECREMENT)
    {
        // Simple decrement
        score = score + POST_DECREMENT;
    }
    return score;
}

int update_nCommenters(int nCommenters, long* commenters_ar, long user_id)
{
    if(commenters_ar==NULL)
    {
        return 0;
    }
    int found=0;
    // The array is big enough to contain the worst case of commenters, i.e., every one comments only once.
    for (int i=0; !found && i<nCommenters;i++)
    {
        if (commenters_ar[i] == user_id)
        {
            found = 1;
        }
    }
    if (!found)
    {
        // The commenter id is new. Save it
        commenters_ar[nCommenters] = user_id;
        nCommenters = nCommenters +1;
    }
    return nCommenters;
}
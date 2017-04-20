#include "process_events.h"

#include "event_list.h"
#include "event_generator.h"

void update_score(int* score, char action_type, char is_comment);


valued_event** process_events(post_block** pb, int size, int * v_event_size)
{
    // For each post block its events are generated and added to a list, then a sorted array is generated from the list
    // Create the list
    event_list* e_list = create_event_list();
    // Process one post block at a time
    int num_events,i,j, score=0;
    for(i=0; i<size; i++)
    {
        event** ee = generate_events(pb[i], &num_events);
        ee = sort_generated_events(ee, num_events);
        for (j=0;j<num_events;j++)
        {
            update_score(&score, ee[j]->type, ee[j]->is_comment);
            if (score <= 0)
            {
                break;
            }
            add_element(e_list,ee[j]->timestamp, ee[j]->post_id, score);
        }
    }
    // Return the array after deleting the list
    valued_event** ve = get_sorted_array(e_list);
    *v_event_size = e_list->size; 
    clear_valued_event_list(e_list);
    return ve;
}

// When the score reaches 0 (or less) should be handled outside this function.
void update_score(int* score, char action_type, char is_comment)
{
    if (action_type==CREATION && !is_comment)
    {
        // Creation of the post
        *score = *score + POST_CREATION;
    }
    else if (action_type==CREATION && is_comment)
    {
        // Creation of a comment
        *score = *score + COMMENT;
    }
    else if (action_type==DECREMENT)
    {
        // Simple decrement
        *score = *score + POST_DECREMENT;
    }
}

#include "event_generator.h"

#include "post_block.h"
#include "debug_utils.h"

#include "stdlib.h"
#include "quicksort.h"

event ** generate_events(post_block * post, int* event_size)
{
   // We have to create 10 events per each post and comment (where 10 is the
   //value assigned at the creation --> it can be modified trhough the apposite define)
   int size = post->comment_ar_size+1;
   long post_id = post->post_id;
   event ** events = malloc(sizeof(event*)*(CREATION+1)*size);
   if (events == NULL)
   {
       print_error("Error in malloc: cannot allocate the space for the events' array");
   }
   // For each post generates 10 events.
   int i,j;
   // Initialize the index for the events
   j=0;
   // Generate the events for the post
   events[j] = create_event(post_id,post->post_ts,CREATION,0);
   j++;
   for(int z=0;z<CREATION;z++) {
       events[j] = create_event(post_id,(post->post_ts)+ONEDAY*(z+1),DECREMENT,0);
       j++;
   }
   // Generate the events for each comment
   for (i=0;i<size-1;i++) {
       events[j] = create_event(post_id,(post->comment_ts)[i],CREATION,1);
       j++;
       for(int z=0;z<CREATION;z++) {
           events[j] = create_event(post_id,(post->comment_ts)[i]+ONEDAY*(z+1),DECREMENT,1);
           j++;
       }
   }
   // Save the size in the dedicated output variable
   *event_size = size*(CREATION+1);
   return events;
}

event ** sort_generated_events(event ** e, int size)
{
    sort_events(e,0,size-1);
    return e;
}

event* create_event(long post_id, int ts, char type, char is_comment) {
    event* e = malloc(sizeof(event*));
    if (e==NULL)
    {
        print_error("Cannot create event");
    }
    e->post_id=post_id;
    e->timestamp=ts;
    e->type=type;
    e->is_comment=is_comment;
    return e;
}

void clear_events(event ** e, int size)
{
    for (int i=0;i<size;i++)
    {
        free(e[i]);
    }
    free(e);
}


event * print_event(event * event)
{
  print_msg("EVENT", "ts: %d, post_id: %ld, type: %d, is_comment: %d", event->timestamp, event->post_id, event->type, event->is_comment);
}

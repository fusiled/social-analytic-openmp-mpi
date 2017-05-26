#ifndef EVENT_GENERATOR_H
#define EVENT_GENERATOR_H

#include "post_block.h"

#define CREATION 10
#define DECREMENT -1
#define EXPIRED 0

#define ONEDAY 60*60*24


/**
* event is produced from post_block and it is, actually an event. When a new post or comment is created
* or when there's a decrement of the score, then there's an event.
**/
typedef struct event
{
    int timestamp;
    int post_ts;
    long user_id;
    long post_id;
    char is_comment;
    int type;
} event;


//create an array of event from an array of post_block
//the size of the new array is saved in event_size
event ** generate_events(post_block * post, int* event_size);


//sort an array of events based on timestamp
event ** sort_generated_events(event ** e, int size);

//create a new event with the specified arguments
event* create_event(long post_id,int post_ts, int ts,long user_id, char type, char is_comment);

//print event to stdout
event * print_event(event * event);

//delete an event array
void clear_events(event ** e, int size);


#endif

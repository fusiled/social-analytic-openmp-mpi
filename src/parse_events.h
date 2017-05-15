#ifndef PARSE_EVENTS
#define PARSE_EVENTS

#include "event_list.h"
#include "top_three.h"


//produce a top_three based on the passed events_array. the size of the output array
//is saved at out_size
top_three ** parse_events(valued_event* events_array, int size, int* out_size);

#endif

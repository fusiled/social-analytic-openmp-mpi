#ifndef PARSE_EVENTS
#define PARSE_EVENTS

#include "event_list.h"
#include "top_three.h"

top_three ** parse_events(valued_event* events_array, int size, int* out_size);

#endif

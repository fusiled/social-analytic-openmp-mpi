#ifndef PROCESS_EVENTS
#define PROCESS_EVENTS

#include "event_list.h"
#include "event_generator.h"

#define POST_CREATION 10
#define COMMENT 10
#define POST_DECREMENT -1

valued_event** process_events(post_block** pb, int size, int * v_event_size);

#endif

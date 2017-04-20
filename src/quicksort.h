#ifndef QUICKSORT
#define QUICKSORT

#include "event_generator.h"
#include "event_list.h"

//sort an array of event. The array contains pointers to event structs.
void sort_events(event * array[], int begin, int end);

void sort_valued_events(valued_event * array[], int begin, int end);

#endif

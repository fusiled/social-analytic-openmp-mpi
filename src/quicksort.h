#ifndef QUICKSORT
#define QUICKSORT

#include "event_generator.h"
#include "event_list.h"

/**
* source of all the sorting routines. all the sort functions are quicksorts
* with an openmp enhancement.
*
*/


//sort an array of event. The array contains pointers to event structs.
void sort_events(event * array[], int begin, int end);

void sort_valued_events(valued_event * array[], int begin, int end);

void sort_valued_events_on_score_with_array(valued_event * array, int begin, int end);
#endif

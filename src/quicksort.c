#include "quicksort.h"
#include "event_generator.h"
#include "event_list.h"


void swap(event * array[], int first_index, int second_index);

void swap_valued_events(valued_event * array[], int first_index, int second_index);

void sort_events(event * array[], int begin, int end)
{
    int pivot, l, r;
    if (end > begin) {
       pivot = array[begin]->timestamp;
       l = begin + 1;
       r = end+1;
       while(l < r)
          if (array[l]->timestamp < pivot)
             l++;
          else {
             r--;
             swap(array,l,r);
          }
       l--;
       swap(array,begin,l);
       sort_events(array, begin, l);
       sort_events(array, r, end);
    }
}

void sort_valued_events(valued_event * array[], int begin, int end)
{
    int pivot, l, r;
    if (end > begin) {
       pivot = array[begin]->post_ts;
       l = begin + 1;
       r = end+1;
       while(l < r)
          if (array[l]->post_ts < pivot)
             l++;
          else {
             r--;
             swap_valued_events(array,l,r);
          }
       l--;
       swap_valued_events(array,begin,l);
       sort_valued_events(array, begin, l);
       sort_valued_events(array, r, end);
    }
}

void swap(event * array[], int first_index, int second_index)
{
    event  * temp = array[first_index];
    array[first_index] = array[second_index];
    array[second_index] = temp;
}

void swap_valued_events(valued_event * array[], int first_index, int second_index)
{
    valued_event * temp = array[first_index];
    array[first_index] = array[second_index];
    array[second_index] = temp;
}

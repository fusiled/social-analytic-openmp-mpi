#include "quicksort.h"
#include "event_generator.h"
#include "event_list.h"

void swap(event * array[], int first_index, int second_index);

void swap_valued_events(valued_event * array[], int first_index, int second_index);

void swap_valued_events_with_array(valued_event * array, int first_index, int second_index);

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

void sort_valued_events_on_score_with_array(valued_event * array, int begin, int end)
{
    int pivot_score, pivot_n_com, l, r;
    long pivot_post_id;
    if (end > begin) {
       pivot_score = array[begin].score;
       pivot_n_com = array[begin].n_commenters;
       pivot_post_id = array[begin].post_id;
       l = begin + 1;
       r = end+1;
       while(l < r)
          if (array[l].score > pivot_score ||
           (array[l].score==pivot_score && array[l].n_commenters > pivot_n_com) ||
           (array[l].score==pivot_score && array[l].n_commenters == pivot_n_com && array[l].post_id<pivot_post_id) )
             l++;
          else {
             r--;
             swap_valued_events_with_array(array,l,r);
          }
       l--;
       swap_valued_events_with_array(array,begin,l);
       sort_valued_events_on_score_with_array(array, begin, l);
       sort_valued_events_on_score_with_array(array, r, end);
    }
}

void swap(event * array[], int first_index, int second_index)
{
    event * temp = array[first_index];
    array[first_index] = array[second_index];
    array[second_index] = temp;
}

void swap_valued_events(valued_event * array[], int first_index, int second_index)
{
    valued_event * temp = array[first_index];
    array[first_index] = array[second_index];
    array[second_index] = temp;
}

void swap_valued_events_with_array(valued_event * array, int first_index, int second_index)
{
    valued_event temp = array[first_index];
    array[first_index] = array[second_index];
    array[second_index] = temp;
}

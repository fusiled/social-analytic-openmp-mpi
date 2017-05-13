#ifndef EVENT_LIST
#define EVENT_LIST

typedef struct valued_event
{
    int post_ts;
    long post_id;
    long user_id;
    int score;
    int n_commenters;
} valued_event;

typedef struct valued_event_list_element
{
    valued_event* v;
    struct valued_event_list_element* next;
} valued_event_list_element;

typedef struct event_list
{
    struct valued_event_list_element * head;
    int size;
} event_list;

// Create an empty list
event_list* create_event_list();

void clear_event_list(event_list * e_list);

void add_element(event_list* list, int post_ts, long post_id,long user_id, int score, int n_commenters);

valued_event** get_sorted_array(event_list* list);

valued_event * merge_valued_event_array_with_ref(valued_event *** ve_arr, int * ve_dim, int ve_size, int * out_size_ref);

valued_event * merge_valued_event_array(valued_event ** ve_arr, int * ve_dim, int ve_size, int * out_size_ref);


void clear_valued_event(valued_event* ve);

void print_valued_event(valued_event * ve);


#endif

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

//Free all the structs associated to the event list BUT NOT THE VALUED_EVENT
//struct. In this way you can easily free space and keep references to pointed
//valued_events
void clear_event_list(event_list * e_list);

//Add a new valued_event to the specified event_list
void add_element(event_list* list, int post_ts, long post_id,long user_id, int score, int n_commenters);


//Return an array of valued_event pointers. The array will be sorted taking into account the timestamp of
//the valued events
valued_event** get_sorted_array(event_list* list);

//Merge arrays of pointers to valued_events pointed by ve_arr. The returned array is sorted by timestamp
//Size of the arrays are kepts into ve_dim. The number of array is contained into ve_size.
//out_size_ref returns the size of the produced array
//NOTE: The array returned has a copy of the elements, not the reference, so it is very important to free it
valued_event * merge_valued_event_array_with_ref(valued_event *** ve_arr, int * ve_dim, int ve_size, int * out_size_ref);

//Merge arrays of valued_events pointed by ve_arr. The returned array is sorted by timestamp
//Size of the arrays are kepts into ve_dim. The number of array is contained into ve_size.
//out_size_ref returns the size of the produced array
//NOTE: The array returned has a copy of the elements, not the reference, so it is very important to free it
valued_event * merge_valued_event_array(valued_event ** ve_arr, int * ve_dim, int ve_size, int * out_size_ref);


//it frees the valued_event ve
void clear_valued_event(valued_event* ve);

//print the valued_event ve to stdout
void print_valued_event(valued_event * ve);


#endif

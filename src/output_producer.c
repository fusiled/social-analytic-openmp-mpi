#include "output_producer.h"


#include "debug_utils.h"
#include "top_three.h"
#include "global_variables.h"

#include "event_list.h"
#include "parse_events.h"

#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

#include <limits.h>

extern const int VALUED_EVENT_NUMBER_TAG;
extern const int VALUED_EVENT_TRANSMISSION_TAG;

int produce_output_file(char *output_file_name, int group_size, MPI_Datatype mpi_valued_event)
{
  //receive the valued_events from all the workers and save the received array into ve_matrix
  //ve_size_ar contains the dimension of the array transmitted from the workers.
  //NOTE that the array transmitted by worker i stays at position i-1
  print_info("Master begins to receive top_threes");
  valued_event ** ve_matrix = malloc(sizeof(valued_event *)*(group_size-1) );
  int * ve_size_ar = calloc(sizeof(int), group_size-1 );
  for(int node_id=1; node_id<group_size; node_id++)
  {
    //it is useless to allocate space to MPI_MASTER
    print_fine("Receiving a valued_event array from worker %d", node_id);
    MPI_Status ret;
    MPI_Recv(ve_size_ar+node_id-1,1,MPI_INT,node_id, VALUED_EVENT_NUMBER_TAG*node_id, MPI_COMM_WORLD, &ret);
    ve_matrix[node_id-1]=malloc(sizeof(valued_event)*ve_size_ar[node_id-1]);
    MPI_Recv(ve_matrix[node_id-1],ve_size_ar[node_id-1],mpi_valued_event,node_id, VALUED_EVENT_TRANSMISSION_TAG*node_id, MPI_COMM_WORLD, &ret);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  print_fine("merging value_event arrays into one");
  int global_ve_size;
  valued_event * global_ve_ar = merge_valued_event_array(ve_matrix, ve_size_ar, group_size-1, &global_ve_size);
  if(global_ve_ar==NULL)
  {
    print_error("Cannot create merged valued_event_array...");
    return -1;
  }
  for(int i=0; i<group_size-1;i++)
  {
    free(ve_matrix[i]);
  }
  free(ve_size_ar);
  print_info("Master begins top_three production phase");
  int output_top_three_size;
  top_three ** output_top_three = parse_events(global_ve_ar, global_ve_size, &output_top_three_size);
  free(global_ve_ar);
  print_info("Output will be composed of %d entries", output_top_three_size);
  print_fine("Finally writing the output file (:");
  //now we produce the output
  FILE * out_fp = fopen(output_file_name,"w"); 
  if(out_fp==NULL)
  {
    print_error("Cannot open output file at path %s", output_file_name);
    return -1;
  }
  for(int i=0; i<output_top_three_size; i++)
  {
    char * output_line = to_string_tuple_top_three(output_top_three[i]);
    free(output_top_three[i]);
    //print_info("writing %s to output_line", output_line);
    fprintf(out_fp, "%s\n", output_line);
    free(output_line);
  }
  free(output_top_three);
  return 0;
}
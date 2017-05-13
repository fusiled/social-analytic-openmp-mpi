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

extern const int TOP_THREE_NUMBER_TAG;
extern const int TOP_THREE_TRANSMISSION_TAG;

//used in produce output file. Decided if quit the loop that produces the output file
unsigned char check_quit_loop(int * tt_counter, int * tt_size_ar, int array_dim);

unsigned char check_quit_loop(int * tt_counter, int * tt_size_ar, int array_dim)
{
  unsigned char quit = 1;
  for(int i=0; i<array_dim; i++)
  {
    if(tt_counter[i]<tt_size_ar[i])
    {
      quit=0;
    }
  }
  return quit;
}


void produce_output_file(char *output_file_name, int group_size, MPI_Datatype mpi_top_three)
{
  //receive the top_three from all the workers and save the received array into tt_matrix
  //tt_size_ar contains the dimension of the array transmitted from the workers.
  //note that the array transmitted by worker i stays at position i-1
  print_info("Master begins to receive top_threes");
  top_three ** tt_matrix = malloc(sizeof(top_three *)*(group_size-1) );
  int * tt_size_ar = calloc(sizeof(int), group_size-1 );
  for(int node_id=1; node_id<group_size; node_id++)
  {
    //it is useless to allocate space to MPI_MASTER
    print_info("Receiving top_three from worker %d", node_id);
    MPI_Status ret;
    MPI_Recv(tt_size_ar+node_id-1,1,MPI_INT,node_id, TOP_THREE_NUMBER_TAG*node_id, MPI_COMM_WORLD, &ret);
    tt_matrix[node_id-1]=malloc(sizeof(top_three)*tt_size_ar[node_id-1]);
    MPI_Recv(tt_matrix[node_id-1],tt_size_ar[node_id-1],mpi_top_three,node_id, TOP_THREE_TRANSMISSION_TAG*node_id, MPI_COMM_WORLD, &ret);
  }
  //now we produce the output
  FILE * out_fp = fopen(output_file_name,"w"); 
  if(out_fp==NULL)
  {
  	print_error("Cannot open output file at path %s", output_file_name);
  	return;
  }
  print_info("Master begins final step: output production");
  //tt_counter keep traces of the top_three that we have to consider along 
  //the arrays contained by tt_matrix
  int tt_size = group_size-1;
  int * tt_counter = calloc(sizeof(int),tt_size);
  top_three * last_tt = NULL;
  unsigned char quit_loop = 0;
  quit_loop = check_quit_loop(tt_counter, tt_size_ar, tt_size);
  while(quit_loop==0)
  {
  	//print_info("New iteration");
    int ts = INT_MAX;
    //get the minor timestamp among tt_matrix[i][counter]
    for(int i=0; i<tt_size; i++)
    {
      int counter = tt_counter[i];
      if(counter<tt_size_ar[i])
      {
        if(tt_matrix[i][counter].ts < ts)
        {
          ts = tt_matrix[i][counter].ts;
        }
      }
    }
    if(ts == INT_MAX)
    {
      quit_loop=1;
      continue;
    }
    //print_info("selected timestamp: %d", ts);
    //fetch the elements from tt_matrix that have the correct ts among tt_matrix[i][counter]
    //and put them into selected_tt
    //print_info("Fetching elements from tt_matrix");
    top_three ** selected_tt = malloc(sizeof(top_three*)*(tt_size) );
    int selected_tt_size = 0;
    for(int i=0; i<tt_size; i++)
    {
      int counter = tt_counter[i];
      if(counter<tt_size_ar[i] && tt_matrix[i][counter].ts==ts)
      {
        selected_tt[selected_tt_size]=&tt_matrix[i][counter];
        selected_tt_size++;
      }
    }
    //print_info("selected_tt_size: %d", selected_tt_size);
    //get the one with the best score among selected_tt
    top_three * tt_winner = NULL;
    if(selected_tt_size==1)
    {
      tt_winner = selected_tt[0];
    }
    else
    if(selected_tt_size>1)
    {
      tt_winner = combine_top_three(selected_tt, selected_tt_size);
    }
    free(selected_tt);
    if(tt_winner!=NULL)
    {
      //if it's major or if it's different than the old one update
      if(compare_top_three_without_timestamp(tt_winner, last_tt)!=0)
      {
        if(compare_top_three_score(tt_winner,last_tt)>0)
        {
          char * output_line = to_string_tuple_top_three(tt_winner);
          //print_info("writing %s to output_line", output_line);
          fprintf(out_fp, "%s\n", output_line);
          free(output_line);
        }
        last_tt=tt_winner;
      }
    }
    //update counters
    for(int i=0; i<tt_size; i++)
    {
      int counter = tt_counter[i];
      if(tt_matrix[i][counter].ts<=ts)
      {
        tt_counter[i]=tt_counter[i]+1;
      }
    }
    quit_loop = check_quit_loop(tt_counter, tt_size_ar, tt_size);
  }
  //free and close everything
  free(tt_counter);
  for(int node_id=1; node_id<group_size; node_id++)
  {
    free(tt_matrix[node_id-1]);
  }
  free(tt_matrix);
  free(tt_size_ar);
  fclose(out_fp);
}


void produce_output_file_event_variant(char *output_file_name, int group_size, MPI_Datatype mpi_valued_event)
{
   //receive the top_three from all the workers and save the received array into tt_matrix
  //tt_size_ar contains the dimension of the array transmitted from the workers.
  //note that the array transmitted by worker i stays at position i-1
  print_info("Master begins to receive top_threes");
  valued_event ** tt_matrix = malloc(sizeof(valued_event *)*(group_size-1) );
  int * tt_size_ar = calloc(sizeof(int), group_size-1 );
  for(int node_id=1; node_id<group_size; node_id++)
  {
    //it is useless to allocate space to MPI_MASTER
    print_info("Receiving valued_event s from worker %d", node_id);
    MPI_Status ret;
    MPI_Recv(tt_size_ar+node_id-1,1,MPI_INT,node_id, TOP_THREE_NUMBER_TAG*node_id, MPI_COMM_WORLD, &ret);
    tt_matrix[node_id-1]=malloc(sizeof(valued_event)*tt_size_ar[node_id-1]);
    MPI_Recv(tt_matrix[node_id-1],tt_size_ar[node_id-1],mpi_valued_event,node_id, TOP_THREE_TRANSMISSION_TAG*node_id, MPI_COMM_WORLD, &ret);
  }
  //now we produce the output
  FILE * out_fp = fopen(output_file_name,"w"); 
  if(out_fp==NULL)
  {
    print_error("Cannot open output file at path %s", output_file_name);
    return;
  }

  //TODO merge events
  print_fine("merging valued events");
  int global_ve_size;
  valued_event * global_ve_ar = merge_valued_event_array(tt_matrix, tt_size_ar, group_size-1, &global_ve_size);
  print_info("Master begins final steps: top_three production");
  int output_top_three_size;
  top_three ** output_top_three = parse_events(global_ve_ar, global_ve_size, &output_top_three_size);
  print_fine("output will be composed of %d entries", output_top_three_size);
  print_fine("Finally writing the output file (:");
  for(int i=0; i<output_top_three_size; i++)
  {
    char * output_line = to_string_tuple_top_three(output_top_three[i]);
    //print_info("writing %s to output_line", output_line);
    fprintf(out_fp, "%s\n", output_line);
    free(output_line);
  }
}
#include "top_three_merge.h"


#include "debug_utils.h"
#include "top_three.h"
#include "global_variables.h"

#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

#include <limits.h>

extern const int TOP_THREE_NUMBER_TAG;
extern const int TOP_THREE_TRANSMISSION_TAG;

//used in produce output file. Decided if quit the loop that produces the output file
unsigned char check_quit_loop_merge(int * tt_counter, int * tt_size_ar, int array_dim);

unsigned char check_quit_loop_merge(int * tt_counter, int * tt_size_ar, int array_dim)
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


top_three * top_three_merge(top_three *** tt_matrix, int * tt_size_ar,int group_size, int * out_tt_size)
{
  int upper_bound_size=0;
  //compute upper bound for the output
  for(int i=0; i<group_size;i++)
  {
    upper_bound_size+=tt_size_ar[i];
  }
  //now we produce the output
  top_three * out_fp = calloc(sizeof(top_three),upper_bound_size);
  if(out_fp==NULL)
  {
  	print_error("Cannot malloc tt_merge");
  	return NULL;
  }
  //tt_counter keep traces of the top_three that we have to consider along 
  //the arrays contained by tt_matrix
  int * tt_counter = calloc(sizeof(int),group_size);
  top_three * last_tt = NULL;
  unsigned char quit_loop = 0;
  quit_loop = check_quit_loop_merge(tt_counter, tt_size_ar, group_size);
  int out_counter=0;
  while(quit_loop==0)
  {
  	//print_info("New iteration");
    int ts = INT_MAX;
    //print_fine("ts: %d", ts);
    //get the minor timestamp among tt_matrix[i][counter]
    for(int i=0; i<group_size; i++)
    {
      int counter = tt_counter[i];
      if(counter<tt_size_ar[i])
      {
        //print_fine("considering top_three:");
        //print_top_three(tt_matrix[i]+counter);
        if(tt_matrix[i][counter]->ts < ts)
        {
          ts = tt_matrix[i][counter]->ts;
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
    top_three ** selected_tt = malloc(sizeof(top_three*)*group_size );
    int selected_tt_size = 0;
    for(int i=0; i<group_size; i++)
    {
      int counter = tt_counter[i];
      if(counter<tt_size_ar[i] && tt_matrix[i][counter]->ts==ts)
      {
        selected_tt[selected_tt_size]=tt_matrix[i][counter];
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
      tt_winner = combine_top_three(selected_tt,selected_tt_size);
    }
    free(selected_tt);
    //print_fine("out writing phase");
    if(tt_winner!=NULL)
    {
      //if it's major or if it's different than the old one update
      if(compare_top_three_without_timestamp(tt_winner, last_tt)!=0)
      {
        if(compare_top_three_score(tt_winner,last_tt)>0)
        {
          out_fp[out_counter]=*tt_winner;
          out_counter++;
        }
        last_tt=tt_winner;
      }
    }
    //update counters
    for(int i=0; i<group_size; i++)
    {
      int counter = tt_counter[i];
      if(counter < tt_size_ar[i] && tt_matrix[i][counter]->ts<=ts)
      {
        tt_counter[i]=tt_counter[i]+1;
      }
    }
    quit_loop = check_quit_loop_merge(tt_counter, tt_size_ar, group_size);
  }
  //free and close everything
  free(tt_counter);
  top_three * out_pointer = realloc(out_fp,sizeof(top_three)*out_counter);
  if(out_pointer==NULL)
  {
    print_error("Cannot malloc out_pointer in top_three_merge. returning NULL");
    return NULL;
  }
  *out_tt_size=out_counter;
  return out_pointer;
}
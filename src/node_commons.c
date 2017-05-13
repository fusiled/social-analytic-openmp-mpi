#include "node_commons.h"


#include <omp.h>
#include "mpi.h"

#include <stdlib.h>

#include "global_variables.h"
#include "debug_utils.h"
#include "top_three.h"
#include "event_list.h"

extern const int MPI_MASTER;


int * get_n_threads_foreach_node(int rank)
{
  int group_size;
  MPI_Comm_size(MPI_COMM_WORLD,&group_size);
  int n_threads = omp_get_max_threads();
  int * n_threads_array=NULL;
  if(rank==MPI_MASTER)
  {	
   n_threads_array = calloc(sizeof(int),group_size);
  }
  MPI_Gather(&n_threads,1,MPI_INT,n_threads_array, 1, MPI_INT, MPI_MASTER, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  return n_threads_array;
}


MPI_Datatype register_mpi_top_three()
{
  int array_of_block_lengths []= {1, 3, 3, 3, 3};
  MPI_Aint  array_of_displacements [5];
  array_of_displacements[0]= offsetof(top_three, ts);
  array_of_displacements[1]= offsetof(top_three, post_id);
  array_of_displacements[2]= offsetof(top_three, user_id);
  array_of_displacements[3]= offsetof(top_three, post_score);
  array_of_displacements[4]= offsetof(top_three, n_commenters);
  MPI_Datatype array_of_types  [] = {MPI_INT, MPI_LONG, MPI_LONG,MPI_INT, MPI_INT};
  MPI_Datatype mpi_top_three;
  int result = MPI_Type_create_struct(5,array_of_block_lengths,array_of_displacements,array_of_types, &mpi_top_three);
  if(result!=MPI_SUCCESS)
  {
    print_error("Error in creating the struct");
  }
  MPI_Type_commit(&mpi_top_three);
  return mpi_top_three;
}

/*int post_ts;
    long post_id;
    long user_id;
    int score;
    int n_commenters;*/
MPI_Datatype register_mpi_valued_event()
{
  int array_of_block_lengths []= {1, 1, 1, 1, 1};
  MPI_Aint  array_of_displacements [5];
  array_of_displacements[0]= offsetof(valued_event, post_ts);
  array_of_displacements[1]= offsetof(valued_event, post_id);
  array_of_displacements[2]= offsetof(valued_event, user_id);
  array_of_displacements[3]= offsetof(valued_event, score);
  array_of_displacements[4]= offsetof(valued_event, n_commenters);
  MPI_Datatype array_of_types  [] = {MPI_INT, MPI_LONG, MPI_LONG,MPI_INT, MPI_INT};
  MPI_Datatype mpi_valued_event;
  int result = MPI_Type_create_struct(5,array_of_block_lengths,array_of_displacements,array_of_types, &mpi_valued_event);
  if(result!=MPI_SUCCESS)
  {
    print_error("Error in creating the struct");
  }
  MPI_Type_commit(&mpi_valued_event);
  return mpi_valued_event; 
}
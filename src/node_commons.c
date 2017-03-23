#include "node_commons.h"


#include <omp.h>
#include "mpi.h"

#include <stdlib.h>

#include "global_variables.h"
#include "debug_utils.h"

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
  print_fine("Getting number of available threads foreach node");
  MPI_Gather(&n_threads,1,MPI_INT,n_threads_array, 1, MPI_INT, MPI_MASTER, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  return n_threads_array;
}

#include "debug_utils.h"

#include "master.h"
#include "worker.h"
#include "global_variables.h"

#include "mpi.h"
#include <stdio.h>
#include <string.h>

/**
* MAIN FILE. Just init the MPI environment, launch master routines or worker 
* routines by observing the rank (the node with rank 0 is the master, the other
* are workers) and finalize MPI environment. Observe master.c and worker.c for
* the details of what a node does.
**/

extern const int MPI_MASTER;

int main(int argc, char *argv[])
{
  int rank;
  int ret_val;
  MPI_Init (&argc,&argv);
  MPI_Comm_rank (MPI_COMM_WORLD,&rank); 
  if(rank==MPI_MASTER)
  {
    ret_val = master_execution(argc, argv);
  }
  else
  {
    ret_val = worker_execution(argc, argv, rank);
  }
  MPI_Finalize();
  return ret_val;
}

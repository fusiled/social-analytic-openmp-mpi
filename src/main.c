
#include "debug_utils.h"

#include "master.h"
#include "worker.h"
#include "global_variables.h"

#include "node_commons.h"


#include "mpi.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

/**
* MAIN FILE. Just init the MPI environment, launch master routines or worker
* routines by observing the rank (the node with rank 0 is the master, the other
* are workers) and finalize MPI environment. Observe master.c and worker.c for
* the details of what a node does.
**/

extern const int MPI_MASTER;


void print_usage(char * exec_name)
{
    printf("%s bucket_size post_file comment_file out_file\n", exec_name);
}


int main(int argc, char *argv[])
{
    if(argc!=5)
    {
        print_error("Wrong number of params!");
        print_usage(argv[0]);
        return -1;
    }
    int provided;
    if(atoi(argv[1])<=0)
    {
        print_error("Invalid bucket size set to %d! ):", atoi(argv[1]) );
    }
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    int rank;
    int ret_val;
    int group_size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&group_size);
    //create mpi_valued_evetn struct and commit it
    MPI_Datatype mpi_valued_event = register_mpi_valued_event();
    //gather at the root the number of available threads foreach node
    int * n_threads_array =  get_n_threads_foreach_node(rank);
    set_debug_level(0);
    if(rank==MPI_MASTER)
    {
        ret_val = master_execution(argc, argv, group_size, n_threads_array, mpi_valued_event);
    }
    else
    {
        ret_val = worker_execution(argc, argv, rank, mpi_valued_event);
    }
    //clear mpi environment
    MPI_Type_free(&mpi_valued_event);
    MPI_Finalize();
    return 0;
}

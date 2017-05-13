#ifndef MASTER_H
#define MASTER_H


#include <mpi.h>

int master_execution(int argc , char * argv[], int group_size, int * n_threads_array, MPI_Datatype mpi_valued_event);

#endif
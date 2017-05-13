#ifndef WORKER_H
#define WORKER_H

#include <mpi.h>

int worker_execution(int argc , char * argv[], int worker_id, MPI_Datatype mpi_top_three, MPI_Datatype mpi_valued_event);

#endif
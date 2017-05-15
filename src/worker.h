#ifndef WORKER_H
#define WORKER_H

#include <mpi.h>


//main routine for worker. See file worker.c for further details
int worker_execution(int argc , char * argv[], int worker_id, MPI_Datatype mpi_valued_event);

#endif
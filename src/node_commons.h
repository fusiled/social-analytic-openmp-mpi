#ifndef NODE_COMMONS_H
#define NODE_COMMONS_H

#include <mpi.h>


//NOT USED
//The master gathers the number of threads of each worker.
//The returned array must be freed
int * get_n_threads_foreach_node();

//Register mpi_valued_event data type
MPI_Datatype register_mpi_valued_event();

#endif
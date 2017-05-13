#ifndef NODE_COMMONS_H
#define NODE_COMMONS_H

#include <mpi.h>

/*
* \function
* \brief gather the number of threads for every node in the system. 
*	Note that only MPI_MASTER will recevie the information of the other nodes
*
**/
int * get_n_threads_foreach_node();

MPI_Datatype register_mpi_top_three();

MPI_Datatype register_mpi_valued_event();

#endif
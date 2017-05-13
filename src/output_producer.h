#ifndef OUTPUT_PRODUCER_H
#define OUTPUT_PRODUCER_H

#include <mpi.h>

//write the final result to output file
void produce_output_file(char *output_file_name, int group_size, MPI_Datatype mpi_top_three);

void produce_output_file_event_variant(char *output_file_name, int group_size, MPI_Datatype mpi_valued_event);

#endif
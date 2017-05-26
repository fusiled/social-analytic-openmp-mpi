#ifndef OUTPUT_PRODUCER_H
#define OUTPUT_PRODUCER_H

#include <mpi.h>

//write the final result to output file. See output_producer.c for the details.
int produce_output_file(char *output_file_name, int group_size, MPI_Datatype mpi_valued_event, int start_ts);

#endif
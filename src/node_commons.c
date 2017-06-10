#include "node_commons.h"


#include <omp.h>
#include "mpi.h"

#include <stdlib.h>

#include "global_variables.h"
#include "debug_utils.h"
#include "event_list.h"

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
    MPI_Gather(&n_threads,1,MPI_INT,n_threads_array, 1, MPI_INT, MPI_MASTER,
               MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    return n_threads_array;
}

/*int post_ts;
    long post_id;
    long user_id;
    int score;
    int n_commenters;*/

MPI_Datatype register_mpi_valued_event()
{
    //block_of_lengths tells the number of elements of the fields of the struct
    //(1 it's just a scalar, >1 is an array)
    int array_of_block_lengths []= {1,1,1,1,1,1,1};
    //array of displacements is the offster between the address fo the struct and
    //the the field. the function offsetoff does all the job we need
    MPI_Aint  array_of_displacements [7];
    array_of_displacements[0]= offsetof(valued_event, valued_event_ts);
    array_of_displacements[1]= offsetof(valued_event, post_ts);
    array_of_displacements[2]= offsetof(valued_event, post_id);
    array_of_displacements[3]= offsetof(valued_event, user_id);
    array_of_displacements[4]= offsetof(valued_event, score);
    array_of_displacements[5]= offsetof(valued_event, n_commenters);
    array_of_displacements[6]= offsetof(valued_event, last_comment_ts);
    //set the data types of the elements of the struct
    MPI_Datatype array_of_types  [] = {MPI_INT,MPI_INT, MPI_LONG, MPI_LONG,MPI_INT, MPI_INT,MPI_INT};
    //define the struct and create the new data struct. This strcut must be committed in
    //a node environment with MPI_Type_commit()
    MPI_Datatype mpi_valued_event;
    int result = MPI_Type_create_struct(7,array_of_block_lengths,array_of_displacements,array_of_types, &mpi_valued_event);
    if(result!=MPI_SUCCESS)
    {
        print_error("Error in creating the struct");
    }
    MPI_Type_commit(&mpi_valued_event);
    return mpi_valued_event;
}
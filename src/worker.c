
#include "worker.h"

#include "global_variables.h"
#include "debug_utils.h"

#include "post_block.h"

#include <stdlib.h>

#include "mpi.h"
#include <omp.h>

extern const int MPI_MASTER;
extern const int POST_NUMBER_TAG;

post_block * receive_post(int worker_id)
{
	MPI_Status ret;
    int * post_ts = malloc(sizeof(int));
    long * post_id = malloc(sizeof(long));
    long * user_id = malloc(sizeof(long));
    int * comment_ar_size = malloc(sizeof(int));
    int * comment_ts=NULL;
    long * comment_user_id=NULL;
	MPI_Recv(post_ts,1,MPI_INT,MPI_MASTER,POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD, &ret);
	//print_fine("Worker %d received post_ts %d", worker_id, *post_ts);
    MPI_Recv(post_id,1,MPI_LONG,MPI_MASTER,POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD, &ret);
    print_fine("Worker %d received post_id %ld", worker_id, *post_id);
    MPI_Recv(user_id,1,MPI_LONG,MPI_MASTER,POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD, &ret);
    //print_fine("Worker %d received user_id %ld", worker_id, *user_id);
    MPI_Recv(comment_ar_size,1,MPI_INT,MPI_MASTER,POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD,&ret);
    //print_fine("Worker %d received comment_ar_size %d", worker_id, *comment_ar_size);
    if( *comment_ar_size >0)
    {
	    comment_ts = calloc(sizeof(int), *comment_ar_size);
	    comment_user_id = calloc(sizeof(long), *comment_ar_size);
	    MPI_Recv(comment_ts,*comment_ar_size,MPI_INT,MPI_MASTER, POST_EXCHANGE_TAG*worker_id*worker_id,MPI_COMM_WORLD, &ret);
	    //print_fine("Worker %d received comment_ts of post %ld", worker_id, *post_id);
	    MPI_Recv(comment_user_id,*comment_ar_size,MPI_LONG, MPI_MASTER, POST_EXCHANGE_TAG*worker_id*worker_id,MPI_COMM_WORLD, &ret);
	    //print_fine("Worker %d received comment_user_id  of post %ld", worker_id, *post_id);
	}
    post_block * ret_pb = new_post_block(*post_ts,*post_id,*user_id,*comment_ar_size,comment_ts,comment_user_id);
    free(post_ts);
    free(post_id);
    free(user_id);
    free(comment_ar_size);
    return ret_pb;
}


int worker_execution(int argc , char * argv[], int worker_id)
{
	int * n_threads = malloc(sizeof(int));
	* n_threads = omp_get_max_threads();
	int * n_threads_array = NULL;
	MPI_Gather(n_threads,1,MPI_INT,n_threads_array, 1, MPI_INT, MPI_MASTER, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Status ret;
	//wait for job reception. If i receive a negative number then i can stop to
	// listen
	int n_posts;
	print_info("Worker %d is waiting for n_posts...", worker_id);
	MPI_Recv(&n_posts,1,MPI_INT, MPI_MASTER, POST_NUMBER_TAG*worker_id,MPI_COMM_WORLD, &ret);
	while(n_posts>0)
	{
		print_info("Worker %d received n_post: %d", worker_id, n_posts);
		for(int i=0; i<n_posts; i++)
		{
			//mpi routine for receive
			post_block * pb = receive_post(worker_id);
			print_info("---->Worker %d Received post_block of post %ld",worker_id,  pb->post_id );
			//free(pb);
		}
		print_info("Worker %d is waiting for n_posts...", worker_id);
		MPI_Recv(&n_posts,1,MPI_INT, MPI_MASTER, POST_NUMBER_TAG*worker_id,MPI_COMM_WORLD, &ret);
	}
	print_msg("STOP SIGNAL", "Worker %d received the stop signal for post trasmission", worker_id);
	//compute top3
	//send top3
	return 0;
}
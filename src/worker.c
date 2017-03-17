
#include "worker.h"

#include "global_variables.h"
#include "debug_utils.h"

#include "post_block.h"

#include <stdlib.h>

#include "mpi.h"

extern const int MPI_MASTER;
extern const int POST_NUMBER_TAG;

post_block * receive_post()
{
	MPI_Status ret;
    int post_ts;
    long post_id;
    long user_id;
    int  comment_ar_size;
    int * comment_ts;
    long * comment_user_id;
	MPI_Recv(&post_ts,1,MPI_INT,MPI_MASTER,POST_EXCHANGE_TAG,MPI_COMM_WORLD, &ret);
    MPI_Recv(&post_id,1,MPI_LONG,MPI_MASTER,POST_EXCHANGE_TAG,MPI_COMM_WORLD, &ret);
    MPI_Recv(&user_id,1,MPI_LONG,MPI_MASTER,POST_EXCHANGE_TAG,MPI_COMM_WORLD, &ret);
    MPI_Recv(&comment_ar_size,1,MPI_INT,MPI_MASTER,POST_EXCHANGE_TAG,MPI_COMM_WORLD,&ret);
    comment_ts = calloc(sizeof(int), comment_ar_size);
    comment_user_id = calloc(sizeof(long), comment_ar_size);
    MPI_Recv(comment_ts,comment_ar_size,MPI_INT,MPI_MASTER, POST_EXCHANGE_TAG,MPI_COMM_WORLD, &ret);
    MPI_Recv(comment_user_id,comment_ar_size,MPI_LONG, MPI_MASTER, POST_EXCHANGE_TAG,MPI_COMM_WORLD, &ret);
    return new_post_block(post_ts,post_id,user_id,comment_ar_size,comment_ts,comment_user_id);
}


int worker_execution(int argc , char * argv[], int worker_id)
{
	MPI_Status ret;
	//wait for job reception. If i receive a negative number then i can stop to
	// listen
	int n_posts;
	MPI_Recv(&n_posts,1,MPI_INT,MPI_MASTER,POST_NUMBER_TAG,MPI_COMM_WORLD, &ret);
	print_info("Worker %d received n_post: %d", worker_id, n_posts);
	//mpi routine for receive
	if(n_posts>0)
	{
		for(int i =0; i<n_posts; i++)
		{
			receive_post();
		}
	}
	else
	{
		//compute top3
		//send top3
	}

	return 0;
}
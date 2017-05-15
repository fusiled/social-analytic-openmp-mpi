
#include "worker.h"

#include "global_variables.h"
#include "debug_utils.h"

#include "post_block.h"
#include "top_three.h"

#include "process_events.h"

#include "event_generator.h"
#include "event_list.h"


#include "parse_events.h"

#include <stdlib.h>

#include <mpi.h>
#include <omp.h>


#define MAX_EVENT_KEEPER_SIZE 2048

extern const int MPI_MASTER;
extern const int POST_NUMBER_TAG;
extern const int POST_EXCHANGE_TAG;
extern const int VALUED_EVENT_NUMBER_TAG;
extern const int VALUED_EVENT_TRANSMISSION_TAG;


//receive a post from MPI_MASTER and return the associated post_block
post_block * receive_post(int worker_id);

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
    //print_fine("Worker %d received post_id %ld", worker_id, *post_id);
    MPI_Recv(user_id,1,MPI_LONG,MPI_MASTER,POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD, &ret);
    //print_fine("Worker %d received user_id %ld", worker_id, *user_id);
    MPI_Recv(comment_ar_size,1,MPI_INT,MPI_MASTER,POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD,&ret);
    //print_fine("Worker %d received comment_ar_size %d", worker_id, *comment_ar_size);
    if( *comment_ar_size >0)
    {
	    comment_ts = calloc(sizeof(int), *comment_ar_size);
	    comment_user_id = calloc(sizeof(long), *comment_ar_size);
	    MPI_Recv(comment_ts,*comment_ar_size,MPI_INT,MPI_MASTER, POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD, &ret);
	    //print_fine("Worker %d received comment_ts of post %ld", worker_id, *post_id);
	    MPI_Recv(comment_user_id,*comment_ar_size,MPI_LONG, MPI_MASTER, POST_EXCHANGE_TAG*worker_id,MPI_COMM_WORLD, &ret);
	    //print_fine("Worker %d received comment_user_id  of post %ld", worker_id, *post_id);

	}
    post_block * ret_pb = new_post_block(*post_ts,*post_id,*user_id,*comment_ar_size,comment_ts,comment_user_id);
    free(post_ts);
    free(post_id);
    free(user_id);
    free(comment_ar_size);
    return ret_pb;
}


int worker_execution(int argc , char * argv[], int worker_id, MPI_Datatype mpi_valued_event)
{
	MPI_Status ret;
	//wait for job reception. If i receive a negative number then i can stop to listen
	int * n_posts = malloc(sizeof(int));
	valued_event *** main_keeper = malloc(sizeof(valued_event **)*MAX_EVENT_KEEPER_SIZE );
	int * main_keeper_dim = malloc(sizeof(int)*MAX_EVENT_KEEPER_SIZE );
	int main_keeper_size=0;
	print_info("Worker %d is waiting for n_posts...", worker_id);
	MPI_Recv(n_posts,1,MPI_INT, MPI_MASTER, POST_NUMBER_TAG*worker_id,MPI_COMM_WORLD, &ret);
	while(*n_posts>=0)
	{
		print_info("Worker %d received n_post: %d", worker_id, *n_posts);
		post_block ** pb_ar = malloc(sizeof(post_block * )*(*n_posts) );
		for(int i=0; i<*n_posts; i++)
		{
			//mpi routine for receive
			pb_ar[i] = receive_post(worker_id);
		}
		print_fine("Worker %d received a post_block. Spawning task");
		#pragma omp parallel
	    #pragma omp single nowait 
	    {
	    	#pragma omp task  shared(worker_id, main_keeper, main_keeper_dim, main_keeper_size)
	    	{
	    		int t_num = omp_get_thread_num();		    		
	    		int v_event_size;
	    		//with this function we generate an array of pointers to valued events.
	    		valued_event** v_event_array =  process_events(pb_ar, *n_posts, &v_event_size);
	    		//save v_event_array into main_keeper. Avoid race conditions with a critical section
	      		#pragma omp critical(MAIN_KEEPER_UPDATE)
      			{
      				main_keeper[main_keeper_size] = v_event_array;
      				main_keeper_dim[main_keeper_size] = v_event_size;
					print_fine("(%d) processed produced a top_three sequence. put it at position %d in main_keeper", worker_id, main_keeper_size);
      				main_keeper_size++;
      			}
				for(int i=0; i<*n_posts; i++)
				{
					del_post_block(pb_ar[i]);
				}
				free(pb_ar);
	    	}
	    }
		print_fine("Worker %d is waiting for n_posts...", worker_id);
		MPI_Recv(n_posts,1,MPI_INT, MPI_MASTER, POST_NUMBER_TAG*worker_id,MPI_COMM_WORLD, &ret);
	}
	#pragma omp barrier
	free(n_posts);
	print_info("Worker %d received the stop signal for post trasmission. main_keeper_size: %d", worker_id, main_keeper_size);
	
	int out_size;
	valued_event * out_ar = merge_valued_event_array_with_ref(main_keeper, main_keeper_dim, main_keeper_size, &out_size);
	print_fine("worker %d produced a valued event array %d big.",worker_id, out_size);
	if(out_ar==NULL)
	{
		print_error("worker %d cannot malloc out_ar", worker_id);
	}
	//simple parallel section. Free memory while sending the valued_event array
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			print_info("Worker %d is sending its valued_events", worker_id);
			MPI_Send(&out_size,1,MPI_INT,MPI_MASTER, VALUED_EVENT_NUMBER_TAG*worker_id, MPI_COMM_WORLD);
			MPI_Send(out_ar,out_size,mpi_valued_event,MPI_MASTER, VALUED_EVENT_TRANSMISSION_TAG*worker_id, MPI_COMM_WORLD);
			free(out_ar);
		}
		#pragma omp section
		{
			print_fine("freeing memory");
			//free main_keeper stuff
			for(int i=0; i<main_keeper_size; i++)
			{
				for(int j=0; j<main_keeper_dim[i];j++)
				{
					clear_valued_event(main_keeper[i][j]);
				}
			}
			free(main_keeper);
			free(main_keeper_dim);
		}
	}
	print_info("Worker %d terminating successfully (:", worker_id);
	return 0;
}
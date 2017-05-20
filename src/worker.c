
#include "worker.h"

#include "global_variables.h"
#include "debug_utils.h"

#include "post_block.h"

#include "process_events.h"

#include "event_generator.h"
#include "event_list.h"

#include "quicksort.h"


#include <stdlib.h>

#include <mpi.h>
#include <omp.h>

#include <string.h>


#define MAX_EVENT_KEEPER_SIZE 2048

extern const int MPI_MASTER;
extern const int POST_NUMBER_TAG;
extern const int POST_EXCHANGE_TAG;
extern const int VALUED_EVENT_NUMBER_TAG;
extern const int VALUED_EVENT_TRANSMISSION_TAG;
extern const int TOP_NUMBER;


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
	print_fine("freeing memory");
	//free main_keeper stuff
	for(int i=0; i<main_keeper_size; i++)
	{
		for(int j=0; j<main_keeper_dim[i];j++)
		{
			clear_valued_event(main_keeper[i][j]);
		}
		free(main_keeper[i]);
	}
	free(main_keeper);
	free(main_keeper_dim);
	//simple parallel section. Free memory while sending the valued_event array
	int counter=0;
	int remaining_events=out_size;
	char stop=0;
	//singal for correct probing
	unsigned int probe= (counter<out_size) ? 1 : 0;
	MPI_Send(&probe,1,MPI_UNSIGNED,MPI_MASTER,VALUED_EVENT_PROBE_TAG*worker_id,MPI_COMM_WORLD);
	print_fine("first element of worker %d has ts: %d", worker_id, out_ar[0].post_ts);
	//print_valued_event(out_ar);
	while(counter<out_size)
	{
		MPI_Status ret;
		int used_elements=0;
		int master_ts;
		//print_fine("Worker %d is at barrier", worker_id);
		//MPI_Recv(&master_ts, 1,MPI_INT, MPI_MASTER, VALUED_EVENT_TS_TAG*worker_id, MPI_COMM_WORLD, &ret);
		MPI_Bcast(&master_ts,1,MPI_INT,MPI_MASTER,MPI_COMM_WORLD);
		//print_fine("Wroker %d got ts: %d", worker_id,master_ts);
		int send_buf_count=0;
		while(counter<out_size && out_ar[counter].post_ts<master_ts)
		{
			counter++;
		}
		for(int i=counter; i<out_size && out_ar[i].post_ts==master_ts; i++)
		{
			send_buf_count++;
		}
		//print_fine("Worker %d had buf count of %d for master_ts=%d", worker_id,send_buf_count, master_ts);
		valued_event * send_buf = malloc(sizeof(valued_event)*send_buf_count);
		memcpy(send_buf,out_ar+counter,sizeof(valued_event)*send_buf_count);
		MPI_Send(&send_buf_count,1,MPI_INT,MPI_MASTER,VALUED_EVENT_NUMBER_TAG*worker_id,MPI_COMM_WORLD);
		if(send_buf_count>0)
		{
			sort_valued_events_on_score_with_array(send_buf, 0, send_buf_count-1);
			if(send_buf_count>TOP_NUMBER)
			{
				send_buf = realloc(send_buf,sizeof(valued_event)*TOP_NUMBER);
				send_buf_count=TOP_NUMBER;
				if(send_buf==NULL)
				{
					print_error("Cannot realloc send_buf in worker %d", worker_id);
				}
			}
			MPI_Send(send_buf,send_buf_count,mpi_valued_event,MPI_MASTER,VALUED_EVENT_TRANSMISSION_TAG*worker_id, MPI_COMM_WORLD);
			//print_fine("Worker %d sent a valued_event array %d big for ts: %d", worker_id, send_buf_count, master_ts);
		}
		free(send_buf);
		counter=counter+send_buf_count;
	}
	int stop_worker_request_code=-1;
	int useless_buf;
	MPI_Bcast(&useless_buf,1,MPI_INT,MPI_MASTER,MPI_COMM_WORLD);
	MPI_Send(&stop_worker_request_code,1,MPI_INT, MPI_MASTER,VALUED_EVENT_NUMBER_TAG*worker_id,MPI_COMM_WORLD);
	free(out_ar);
	print_info("Worker %d terminating successfully (:", worker_id);
	return 0;
}

#include "worker.h"

#include "global_variables.h"
#include "debug_utils.h"

#include "post_block.h"
#include "top_three.h"

#include "process_events.h"

#include "event_generator.h"
#include "event_list.h"


#include "parse_events.h"

#include "top_three_merge.h"

#include <stdlib.h>

#include <mpi.h>
#include <omp.h>


#define MAX_EVENT_KEEPER_SIZE 2048

extern const int MPI_MASTER;
extern const int POST_NUMBER_TAG;
extern const int POST_EXCHANGE_TAG;
extern const int TOP_THREE_NUMBER_TAG;
extern const int TOP_THREE_TRANSMISSION_TAG;


//receive a post from MPI_MASTER and return the associated post_block
post_block * receive_post(int worker_id);

void resize_keeper(top_three **** keeper, int ** keeper_dim, int keeper_size);

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


int worker_execution(int argc , char * argv[], int worker_id, MPI_Datatype mpi_top_three)
{
	MPI_Status ret;
	//wait for job reception. If i receive a negative number then i can stop to
	// listen
	int * n_posts = malloc(sizeof(int));
	top_three *** main_keeper = malloc(sizeof(top_three **)*MAX_EVENT_KEEPER_SIZE );
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
			//print_info("---->Worker %d Received post_block of post %ld",worker_id,  pb->post_id );
		}
		print_info("Worker %d received a post_block. Spawning task");
		#pragma omp parallel
	    #pragma omp single nowait 
	    {
	    	#pragma omp task  shared(worker_id)
	    	{
	    		int t_num = omp_get_thread_num();		    		
	    		int v_event_size;
	    		print_fine("pb_ar at %p", pb_ar);
	    		valued_event** v_event_array =  process_events(pb_ar, *n_posts, &v_event_size);
	    		print_fine("(%d,%d) processed a post_block batch", worker_id, t_num);
				//compute top_three for current v_event_array
				int output_top_three_size;
				top_three ** output_top_three = parse_events(v_event_array, v_event_size, &output_top_three_size);
				//valued events now are useless. Free space!
				print_info("freeing unused stuff");
				for(int i=0; i<v_event_size;i++)
				{
					clear_valued_event(v_event_array[i]);
				}
				free(v_event_array);
	    		//free post_block array
				for(int i=0; i<*n_posts; i++)
				{
					del_post_block(pb_ar[i]);
				}
				free(pb_ar);

				/*for(int i=0; i<output_top_three_size;i++)
				{
					print_top_three(output_top_three[i]);
				}*/
	      		//add output_top_three array to the main_keeper
	      		#pragma omp critical 
      			{
      				main_keeper[main_keeper_size] = output_top_three;
      				main_keeper_dim[main_keeper_size] = output_top_three_size;
					print_fine("(%d,%d) processed produced a top_three sequence. put it at position %d in main_keeper", worker_id, t_num, main_keeper_size);
      				main_keeper_size++;
      			}
				//clear_event_list(e_list);
	    	}
	    }
		print_info("Worker %d is waiting for n_posts...", worker_id);
		MPI_Recv(n_posts,1,MPI_INT, MPI_MASTER, POST_NUMBER_TAG*worker_id,MPI_COMM_WORLD, &ret);
	}
	#pragma omp barrier
	free(n_posts);
	print_info("Worker %d received the stop signal for post trasmission. main_keeper_size: %d", worker_id, main_keeper_size);
	//resize keeper stuff
	//resize_keeper(&main_keeper,&main_keeper_dim,main_keeper_size);
	
	/*for(int i=0; i<main_keeper_size;i++)
	{
		for(int j=0; j<main_keeper_dim[i];j++)
		{
			print_top_three(main_keeper[i][j]);
		}
	}*/
	//use parse events on single entries of main_keeper, then reuse the 
	//code in output_producer to merge the top_three. Then send everything to master
	int out_size=0;
	print_info("main_keeper_size: %d", main_keeper_size);
	top_three * out_ar = top_three_merge(main_keeper,main_keeper_dim,main_keeper_size,&out_size);
	/*for(int i=0; i<out_size;i++)
	{
		print_top_three(out_ar+i);
	}
	//compute top3
	/*long pid[]={1,2,3};
	long uid[]={4,5,6};
	int score[]={10,20,30};
	int nc[]={7,8,9};
	top_three * top_three_ar = new_top_three(worker_id,pid,uid,score,nc);
	int top_three_ar_size = 1;*/
	//top_three * out_ar = NULL;
	print_info("Worker %d is sending its top_three", worker_id);
	MPI_Send(&out_size,1,MPI_INT,MPI_MASTER, TOP_THREE_NUMBER_TAG*worker_id, MPI_COMM_WORLD);
	MPI_Send(out_ar,out_size,mpi_top_three,MPI_MASTER, TOP_THREE_TRANSMISSION_TAG*worker_id, MPI_COMM_WORLD);

	//FINAL CLEANUP
	//free main_keeper stuff
	for(int i=0; i<main_keeper_size; i++)
	{
		for(int j=0; j<main_keeper_dim[i];j++)
		{
			del_top_three(main_keeper[i][j]);
		}
	}
	free(main_keeper);
	free(main_keeper_dim);

	return 0;
}



void resize_keeper(top_three **** keeper, int ** keeper_dim, int keeper_size)
{
	//free space
	if(keeper_size==0)
	{
		//free unused space as soon as we can. To avoid errors on future free 
		//also set the returning pointers to null
		free(*keeper);
		free(*keeper_dim);
		*keeper=NULL;
		*keeper_dim=NULL;
	}
	//else resize
	else if(keeper_size>0)
	{
		top_three *** keeper_buffer = realloc(*keeper,sizeof(top_three**)*keeper_size);
		if(keeper_buffer==NULL)
		{
			print_warning("Error in main_keeper resizing. Keeping the old one");
		}
		else
		{
			*keeper = keeper_buffer;
		}
		int * keeper_dim_buffer = realloc(*keeper_dim,sizeof(int)*keeper_size);
		if(keeper_dim_buffer==NULL)
		{
			print_warning("Error in main_keeper_dim resizing. Keeping the old one");
		}
		else
		{
			*keeper_dim = keeper_dim_buffer;
		}
	}	
}
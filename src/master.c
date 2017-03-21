#include "master.h"


#include "debug_utils.h"
#include "reply_type.h"
#include "khash.h"
#include "comment.h"
#include "post.h"
#include "parser.h"
#include "global_variables.h"
#include "utils.h"


#include "mpi.h"
#include <omp.h>

#define BUCKET_SIZE 200000


extern const reply_type COMMENT_REPLY_TYPE;
extern const reply_type POST_REPLY_TYPE;

extern const int POST_EXCHANGE_TAG;
extern const int POST_NUMBER_TAG;

extern const int MPI_MASTER;


//run these macros for hashmaps initializations
KHASH_MAP_INIT_INT64(comment_post_hashmap, long)
KHASH_MAP_INIT_INT64(post_comment_list_hashmap, comment_list *)

khash_t(comment_post_hashmap) *cph;
khash_t(post_comment_list_hashmap) *pclh;

//send the array pointed by post_ar (that is big post_ar_size) to node_id
void send_posts_to_node(post * post_ar,int * post_ar_size, int node_id);

int master_execution(int argc, char * argv[])
{
  int group_size;
  MPI_Comm_size(MPI_COMM_WORLD,&group_size);
  int n_threads = omp_get_max_threads();
  int * n_threads_array = calloc(sizeof(int),group_size);
  print_fine("Getting number of available threads foreach node");
  MPI_Gather(&n_threads,1,MPI_INT,n_threads_array, 1, MPI_INT, MPI_MASTER, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  print_int_array(n_threads_array, group_size);
  FILE * post_fp = fopen(argv[1],"r");
  FILE * comm_fp = fopen(argv[2],"r"); 
  print_info("Files opened correctly");
  int * read_lines;
  read_lines = malloc(sizeof(int));
  
  //init hashmaps
  cph = kh_init(comment_post_hashmap); // cph[comment_id] -> post_id
  pclh = kh_init(post_comment_list_hashmap); //pclh[post_id] -> comment_list
  //fetch comment
  comment * comm = parse_comment(comm_fp,1, read_lines);
  //iterate comm_fp. the while will fill both cph and pclh
  while (comm!=NULL)
  {
    //guard against spurious end
    if(comm->comm_id!=0)
    {
      khiter_t k;
      long post_key;
      int ret, is_missing;
      //check if reply_type is COMMENT_REPLY_TYPE or POST_REPLY_TYPE
      if(comm->repl_ty == COMMENT_REPLY_TYPE)
      {
        //if is a reply to a comment, then we must understand to which post this
        //comment is related. We can know this because cph maps comments to the
        // related post
        long post_key;
        k = kh_get(comment_post_hashmap, cph, comm->reply_id);
        if( k==kh_end(cph) )
        {
          print_error("%ld is missing in key of cph", comm->reply_id);
        }
        post_key = kh_value(cph,k); // post_key = cph[reply_id]
        //print_info("COMMENT TYPE. cph[%ld]=%ld", comm->reply_id, kh_value(cph,k) );
        //insert new value cph : cph[comm_id] = post_key
        k = kh_put(comment_post_hashmap, cph, comm->comm_id, &ret);
        kh_value(cph,k) = post_key;
        //print_info("COMMENT TYPE. cph[%ld]=%ld", comm->comm_id, kh_value(cph,k) );
      }
      else
      {
        //this is a reply to a post, so just add cph[comm_id]= reply_id
        post_key = comm->reply_id;
        //set comment_post_hashmap[comm->comm_id] = comm->reply id in case of post reply
        k = kh_put(comment_post_hashmap, cph, comm->comm_id, &ret);
        kh_value(cph,k)=comm->reply_id;
        //print_info("POST_TYPE. new assignment: cph[%ld]= %ld", comm->comm_id, kh_value(cph,k));
      }
      //check if there are errors
      if(post_key==0 || comm->comm_id==0)
      {
        print_error("post_key or comm->comm_id error: cph[%ld]=%ld", comm->comm_id, post_key);
        print_comment(comm);
      }
      //add comment to a comment_list associated to a post. pclh maps post to comment_list
      k = kh_get(post_comment_list_hashmap, pclh, post_key);
      is_missing = (k == kh_end(pclh));   
      // if the key is missing then create en empty comment_list: pclh[post_key]=new_comment_list()
      if(is_missing)
      {
        k = kh_put(post_comment_list_hashmap, pclh, post_key, &ret); 
        kh_value(pclh,k)=new_comment_list();
        //print_info("Adding new comment list for post %ld", post_key);
      }
      //get comment_list and add comment to it
      comment_list * comm_list = kh_value(pclh,k);
      add_to_comment_list(comm_list,comm);
      //print_info("Added comment with id %ld to comment_list of post %ld", comm->comm_id, post_key);
    }
    comm = parse_comment(comm_fp,1, read_lines);
  }
  post * post_ar = parse_post(post_fp,BUCKET_SIZE, read_lines);
  int node_id=2;
  while(post_ar!=NULL)
  {
    //#pragma omp parallel private(node_id)
    //#pragma omp single nowait 
    //{
    //  #pragma omp task
      /*{
        //send_posts_to_node(post_ar, read_lines, 1); 
        free(post_ar);
      }*/
    //}
    send_posts_to_node(post_ar, read_lines, node_id);
    //print_info("Post_ar ref is %p", post_ar);
    del_post(post_ar);
    post_ar = parse_post(post_fp,BUCKET_SIZE, read_lines); 
    print_info("update node_id");
    do
    {
      node_id = (node_id+1)%group_size;
    } while(node_id == MPI_MASTER);
  }
  print_info("STOP EVERYTHING");
  for (int i=0; i<group_size; i++)
  {
    if(i == MPI_MASTER )
    {
      continue;
    }
      int stop_code = -10;
      MPI_Send(&stop_code, 1, MPI_INT, i, POST_NUMBER_TAG*i, MPI_COMM_WORLD);
      print_msg("NEXT","sent stop code to node %d", i);
  }
  print_info("Master finished sending stop post messages");
  //close everything
  free(n_threads_array);
  free(read_lines);
  kh_destroy(post_comment_list_hashmap, pclh);
  kh_destroy(comment_post_hashmap, cph);
  fclose(post_fp);
  fclose(comm_fp);
  return 0;
}


void send_posts_to_node(post * post_ar,int * post_ar_size, int node_id){
  //notify worker: we send it the number of posts that master will transmit
  MPI_Send(post_ar_size,1,MPI_INT,node_id,POST_NUMBER_TAG*node_id,MPI_COMM_WORLD);
  //send to node the number of posts that it will receive
  print_info("Sent post reception signal of %d to worker %d", *post_ar_size, node_id);
  for(int i=0; i < (*post_ar_size); i++)
  {
    post * current_post = post_ar+i;
    //print_fine("Current post:");
    //print_post(current_post);
    //fetch comment list
    khiter_t k;
    int ret, is_missing;
    k = kh_get(post_comment_list_hashmap, pclh, current_post->post_id);
    is_missing = (k == kh_end(pclh));   
    // if the key is missing then create en empty comment_list: pclh[post_key]=new_comment_list()
    if(is_missing)
    {
      k = kh_put(post_comment_list_hashmap, pclh, current_post->post_id, &ret); 
      kh_value(pclh,k)=new_comment_list();
      //print_info("Adding new comment list for post %ld", current_post->post_id);
    }
    comment_list * comm_list = kh_value(pclh,k);
    int * post_ts = &(current_post->ts);
    long * post_id = &(current_post->post_id);
    long * user_id = &(current_post->user_id);
    //get comment list from pclh
    //build arrays that will be sent from comment_list
    int * comment_ts;
    long * comment_user_id;
    int * comment_ar_size = malloc(sizeof(int));
    if(comment_ar_size==NULL)
    {
      print_error("Cannot Malloc comment_ar_size");
      return;
    }
    build_arrays_from_comment_list(comm_list,comment_ts, comment_user_id, comment_ar_size);
    //send stuff to node_id
    //print_info("Master starts to send stuff of post %ld", *post_id);
    MPI_Send(post_ts,1,MPI_INT,node_id,POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
    //print_fine("Master: sent post_ts %d", *post_ts);
    MPI_Send(post_id,1,MPI_LONG,node_id,POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
    //print_fine("Master: sent post_id %ld", *post_id);
    MPI_Send(user_id,1,MPI_LONG,node_id,POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
    //print_fine("Master: sent user_id %ld", *user_id);
    MPI_Send(comment_ar_size, 1, MPI_INT, node_id, POST_EXCHANGE_TAG*node_id, MPI_COMM_WORLD);
    //print_fine("Master: sent comment_ar_size %d", *comment_ar_size);
    if(*comment_ar_size>0)
    {
      //print_info("Master transmists arrays");
      MPI_Send(comment_ts,*comment_ar_size, MPI_INT,node_id, POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
      //print_fine("Master: sent comment_ts of post %ld", *post_id);
      MPI_Send(comment_user_id,*comment_ar_size,MPI_LONG,node_id, POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
      //print_fine("Master: sent comment_user_id of post %ld", *post_id);
    }
    //print_info("Master finished transmission of post %ld", *post_id);
    free(comment_ar_size);
    //del_comment_list(comm_list);
    //free(comment_user_id);
    //free(comment_ts);
  }
}
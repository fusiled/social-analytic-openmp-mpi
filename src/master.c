#include "master.h"


#include "debug_utils.h"
#include "reply_type.h"
#include "khash.h"
#include "comment.h"
#include "post.h"
#include "parser.h"


#include "mpi.h"

#define BUCKET_SIZE 10

const int CAP = 10;


extern const reply_type COMMENT_REPLY_TYPE;
extern const reply_type POST_REPLY_TYPE;

extern const int POST_EXCHANGE_TAG;
extern const int POST_NUMBER_TAG;


//run these macros for hashmaps initializations
KHASH_MAP_INIT_INT64(comment_post_hashmap, long)
KHASH_MAP_INIT_INT64(post_comment_list_hashmap, comment_list *)


void send_posts_to_node(post * post_ar,int post_ar_size, int node_id);

void print_usage(char * exec_name)
{
  printf("%s post_file comment_file out_file\n", exec_name);
}

int master_execution(int argc, char * argv[])
{
  if(argc!=4)
  {
    print_error("Wrong number of params!");
    print_usage(argv[0]);
    return -1;
  }
  int group_size;
  MPI_Comm_size(MPI_COMM_WORLD,&group_size);
  FILE * post_fp = fopen(argv[1],"r");
  FILE * comm_fp = fopen(argv[2],"r"); 
  print_info("Files opened correctly");
  int read_lines;
  
  //init hashmaps
  khash_t(comment_post_hashmap) *cph = kh_init(comment_post_hashmap); // cph[comment_id] -> post_id
  khash_t(post_comment_list_hashmap) *pclh = kh_init(post_comment_list_hashmap); //pclh[post_id] -> comment_list
  //fetch comment
  comment * comm = parse_comment(comm_fp,1, &read_lines);
  int counter =0; //XXX TEST COUNTER
  //iterate comm_fp. the while will fill both cph and pclh
  //TODO evaluate a file processing splitting with openmp
  while (comm!=NULL && counter < CAP )
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
        print_info("COMMENT TYPE. cph[%ld]=%ld", comm->reply_id, kh_value(cph,k) );
        //insert new value cph : cph[comm_id] = post_key
        k = kh_put(comment_post_hashmap, cph, comm->comm_id, &ret);
        kh_value(cph,k) = post_key;
        print_info("COMMENT TYPE. cph[%ld]=%ld", comm->comm_id, kh_value(cph,k) );
      }
      else
      {
        //this is a reply to a post, so just add cph[comm_id]= reply_id
        post_key = comm->reply_id;
        //set comment_post_hashmap[comm->comm_id] = comm->reply id in case of post reply
        k = kh_put(comment_post_hashmap, cph, comm->comm_id, &ret);
        kh_value(cph,k)=comm->reply_id;
        print_info("POST_TYPE. new assignment: cph[%ld]= %ld", comm->comm_id, kh_value(cph,k));
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
        print_info("Adding new comment list for post %ld", post_key);
      }
      //get comment_list and add comment to it
      comment_list * comm_list = kh_value(pclh,k);
      add_to_comment_list(comm_list,comm);
      print_info("Added comment with id %ld to comment_list of post %ld", comm->comm_id, post_key);
    }
    comm = parse_comment(comm_fp,1, &read_lines);
    counter ++; //XXX TEST COUNTER
  }
  /*post * post_ar = parse_post(post_fp,BUCKET_SIZE, &read_lines);
  int node_id=0;
  while(post_ar!=NULL)
  {
    send_posts_to_node(post_ar, read_lines, node_id); 
    free(post_ar);
    node_id=( (node_id+1) %group_size);
    post_ar = parse_post(post_fp,BUCKET_SIZE, &read_lines); 
  }  */
  //TEST
  post * post_ar = parse_post(post_fp,BUCKET_SIZE, &read_lines);
  send_posts_to_node(post_ar, read_lines, 1); 
  free(post_ar);

  //close everything
  kh_destroy(post_comment_list_hashmap, pclh);
  kh_destroy(comment_post_hashmap, cph);
  fclose(post_fp);
  fclose(comm_fp);
  return 0;
}


void send_posts_to_node(post * post_ar,int post_ar_size, int node_id){
  //notify worker: we send it the number of posts that master will transmit
  MPI_Send(&post_ar_size,1,MPI_INT,node_id,POST_NUMBER_TAG,MPI_COMM_WORLD);
  print_info("Sent post reception signal of %d to worker %d", post_ar_size, node_id);
  //get pclh
  khash_t(post_comment_list_hashmap) *pclh = kh_init(post_comment_list_hashmap); //pclh[post_id] -> comment_list
  //send to node the number of posts that it will receive
  for(int i=0; i<post_ar_size;i++)
  {
    post * current_post = post_ar+i;
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
      print_info("Adding new comment list for post %ld", current_post->post_id);
    }
    comment_list * comm_list = kh_value(pclh,k);
    int * post_ts = &(current_post->ts);
    long * post_id = &(current_post->post_id);
    long * user_id = &(current_post->user_id);
    //get comment list from pclh
    //build arrays that will be sent from comment_list
    int * comment_ts;
    long * comment_user_id;
    int comment_ar_size;
    build_arrays_from_comment_list(comm_list,comment_ts, comment_user_id, &comment_ar_size);
    //send stuff to node_id
    MPI_Send(post_ts,1,MPI_INT,node_id,POST_EXCHANGE_TAG,MPI_COMM_WORLD);
    MPI_Send(post_id,1,MPI_LONG,node_id,POST_EXCHANGE_TAG,MPI_COMM_WORLD);
    MPI_Send(user_id,1,MPI_LONG,node_id,POST_EXCHANGE_TAG,MPI_COMM_WORLD);
    MPI_Send(&comment_ar_size, 1, MPI_INT, node_id, POST_EXCHANGE_TAG, MPI_COMM_WORLD);
    MPI_Send(comment_ts,comment_ar_size, MPI_INT,node_id, POST_EXCHANGE_TAG,MPI_COMM_WORLD);
    MPI_Send(comment_user_id,comment_ar_size,MPI_LONG,node_id, POST_EXCHANGE_TAG,MPI_COMM_WORLD);
  }
}
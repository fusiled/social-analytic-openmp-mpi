#include "master.h"

//src includes
#include "khash.h"
#include "debug_utils.h"
#include "reply_type.h"
#include "comment.h"
#include "post.h"
#include "parser.h"
#include "global_variables.h"

#include "top_three.h"


//library includes
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include <string.h>

#define MAX_OUTPUT_NAME_SIZE 256

#define BUCKET_SIZE 1000000
// the number of post in the big dataset is 3594403

//from reply_type.h
extern const reply_type COMMENT_REPLY_TYPE;
extern const reply_type POST_REPLY_TYPE;

extern const int POST_EXCHANGE_TAG;
extern const int POST_NUMBER_TAG;

extern const int MPI_MASTER;
extern const int STOP_POST_TRANSMISSION_SIGNAL;


//run these macros for hashmaps initializations
KHASH_MAP_INIT_INT64(comment_post_hashmap, long)
KHASH_MAP_INIT_INT64(post_comment_list_hashmap, comment_list *)


//global variable for Post to Comment List Hashmap
//pclh maps post_id to a comment_list
khash_t(post_comment_list_hashmap) * pclh;


//parse the comment file and build pclh
void build_post_to_comment_list_hashmap(char * path_to_comment_file);

//send the array pointed by post_ar (that is big post_ar_size) to node_id and
//the related comment list. The worker will recevied the combination of data and
//will build a post block foreach post
void transmit_post_block_array_to_node(post * post_ar,int * post_ar_size, int node_id);


//process to post file and send block of posts (at most big BUCKET_SIZE) to the workers
void process_posts_and_transmit(char * path_to_post_file, int group_size);


//update the node_id. This is used to decide to who send a bucket of posts.
int update_node_id(int node_id, int group_size);

//write the final result to output file
void produce_output_file(char *output_file_name);

int update_node_id(int node_id, int group_size)
{
    do
    {
      node_id = (node_id+1)%group_size;
    } 
    while(node_id == MPI_MASTER);
    print_info("node_id updated to %d", node_id);
    return node_id;
}

int master_execution(int argc, char * argv[], int group_size, int * n_threads_array, MPI_Datatype mpi_top_three)
{
  //argv[2] is the path to the comment file
  //argv[1] is the path to the post file
  time_t seconds = time(NULL);
  char output_file_name [ MAX_OUTPUT_NAME_SIZE ];
  char buffer [ MAX_OUTPUT_NAME_SIZE ];
  output_file_name[0]='\0';
  strcat(output_file_name,argv[3]);
  snprintf(buffer,sizeof(char)*MAX_OUTPUT_NAME_SIZE-strlen(argv[3]),"%d",seconds);
  strcat(output_file_name, "_");
  strcat(output_file_name, buffer);
  print_msg("OUTPUT","The output will be saved at file %s", output_file_name);
  //init pclh
  pclh = kh_init(post_comment_list_hashmap); //pclh[post_id] -> comment_list

  build_post_to_comment_list_hashmap(argv[2]);

  process_posts_and_transmit(argv[1], group_size);

  //pclh is no longer needed
  kh_destroy(post_comment_list_hashmap, pclh);

  produce_output_file(output_file_name);
  
  
  free(n_threads_array);
  return 0;
}


void transmit_post_block_array_to_node(post * post_ar,int * post_ar_size, int node_id){
  //notify worker: we send it the number of post blocks that master will transmit
  print_info("Sending post reception signal of %d to worker %d", *post_ar_size, node_id);
  MPI_Send(post_ar_size,1,MPI_INT,node_id,POST_NUMBER_TAG*node_id,MPI_COMM_WORLD);
  //send to node the number of posts that it will receive
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
    // if the key is missing then comment_list = null
    comment_list * comm_list;
    if(is_missing)
    {
      comm_list = NULL;
    }
    else
    {
      comm_list = kh_value(pclh,k);
    }
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
      print_error("Cannot malloc comment_ar_size");
      return;
    }
    build_arrays_from_comment_list(comm_list,&comment_ts, &comment_user_id, comment_ar_size);
    //send stuff to node_id
    //print_info("Master starts to send stuff of post %ld", *post_id);
    MPI_Send(post_ts,1,MPI_INT,node_id,POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
    //print_fine("Master: sent post_ts %d", *post_ts);
    MPI_Send(post_id,1,MPI_LONG,node_id,POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
    //print_fine("Master: sent post_id %ld", *post_id);
    MPI_Send(user_id,1,MPI_LONG,node_id,POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
    //print_fine("Master: sent user_id %ld", *user_id);
    MPI_Send(comment_ar_size, 1, MPI_INT, node_id, POST_EXCHANGE_TAG*node_id, MPI_COMM_WORLD);
    //print_fine("Master: sent comment_ar_size %d at addres %p", *comment_ar_size, comment_ar_size);
    if(*comment_ar_size>0)
    {
      //print_info("Master transmists arrays");
      //print_fine("Master: sending comment_ts of post %ld at address %d", *post_id, comment_ts);
      MPI_Send(comment_ts,*comment_ar_size, MPI_INT,node_id, POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
      //print_fine("Master: sending comment_user_id of post %ld at address %d", *post_id, comment_user_id);
      MPI_Send(comment_user_id,*comment_ar_size,MPI_LONG,node_id, POST_EXCHANGE_TAG*node_id,MPI_COMM_WORLD);
      free(comment_user_id);
      free(comment_ts);
      del_comment_list(comm_list);
    }
    //print_info("Master finished transmission of post %ld", *post_id);
    free(comment_ar_size);
  }
}



void build_post_to_comment_list_hashmap(char * path_to_comment_file)
{
  FILE * comm_fp = fopen(path_to_comment_file,"r"); 
  int read_lines;
  //fetch comment
  comment * comm = parse_comment(comm_fp,1, &read_lines);
  //iterate comm_fp. the while will fill both cph and pclh
  khash_t(comment_post_hashmap) *cph = kh_init(comment_post_hashmap); // cph[comment_id] -> post_id
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
    comm = parse_comment(comm_fp,1, &read_lines);
  }
  //close comment file
  fclose(comm_fp);
  kh_destroy(comment_post_hashmap, cph);
}


void process_posts_and_transmit(char * path_to_post_file, int group_size)
{
  FILE * post_fp = fopen(path_to_post_file,"r");
  int * read_lines;
  read_lines = malloc(sizeof(int));
  post * post_ar = parse_post(post_fp,BUCKET_SIZE, read_lines);
  int node_id=0;
  while(post_ar!=NULL)
  {
    node_id=update_node_id(node_id, group_size);
    transmit_post_block_array_to_node(post_ar, read_lines, node_id);
    //print_info("Post_ar ref is %p", post_ar);
    del_post(post_ar);
    post_ar = parse_post(post_fp,BUCKET_SIZE, read_lines); 
  }
  free(read_lines);
  fclose(post_fp);
  for (int i=0; i<group_size; i++)
  {
    if(i == MPI_MASTER )
    {
      continue;
    }
    MPI_Send(&STOP_POST_TRANSMISSION_SIGNAL, 1, MPI_INT, i, POST_NUMBER_TAG*i, MPI_COMM_WORLD);
  }
  print_info("Signaled the end of post transmission to all the nodes");
}


void produce_output_file(char *output_file_name)
{
  //TODO make implementation
}
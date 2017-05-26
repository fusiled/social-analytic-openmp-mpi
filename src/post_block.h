#ifndef POST_BLOCK_H
#define POST_BLOCK_H


/**
* This data structure is what the workers must handle as input from
* master.
**/

typedef struct post_block
{
	int post_ts; //ts of the post
	long post_id; // id of the post
    long user_id; // id of the user
    int  comment_ar_size; // the size of comment_ts and comment_user_id arrays
    //NOTE: comments_ts[i] is correlated to comment_user_id[i], Which means they are related
    //to the same comment.
    int * comment_ts; //an array that contains the timestamp of the comments related to the post
    long * comment_user_id; //an array that contains ids of the user of the comments related to the post
} post_block;


//create a new post_block. This is just a wrapper for a malloc and some assignments
post_block * new_post_block(int post_ts, long post_id, long user_id,
		int comment_ar_size, int * comment_ts, long * comment_user_id);


//delete the passed post_block. NOTE: the arrays pointed by the struct are also destroyed
void del_post_block(post_block * pb);


#endif
#ifndef POST_BLOCK_H
#define POST_BLOCK_H


typedef struct post_block_struct
{
	int post_ts;
	long post_id;
    long user_id;
    int  comment_ar_size;
    int * comment_ts;
    long * comment_user_id;
} post_block;


post_block * new_post_block(int post_ts, long post_id, long user_id,
		int comment_ar_size, int * comment_ts, long * comment_user_id);


void del_post_block(post_block * pb);


#endif
#include "post_block.h"


#include <stdlib.h>

#include "debug_utils.h"


post_block * new_post_block(int post_ts, long post_id, long user_id,
		 int comment_ar_size, int * comment_ts, long * comment_user_id)
{
	post_block * ret_pb = malloc(sizeof(post_block));
	if(ret_pb==NULL)
	{
		print_error("Cannot allocate a new post_block");
		return NULL;
	}
	ret_pb->post_ts=post_ts;
	ret_pb->post_id=post_id;
	ret_pb->user_id=user_id;
	ret_pb->comment_ar_size = comment_ar_size;
	ret_pb->comment_ts=comment_ts;
	ret_pb->comment_user_id = comment_user_id;
	//print_info("Created a new post_block with comment_ar_size: %d", ret_pb->comment_ar_size);
	return ret_pb;
}


void del_post_block(post_block * pb)
{
	//print_info("free post_block");
	free(pb->comment_ts);
	//print_info("freed comment_ts");
	free(pb->comment_user_id);
	//print_info("freed comment_user_id");
	free(pb);
}
#include "post.h"

#include "comment_list.h"
#include "debug_utils.h"
#include "khash.h"

#include <stdlib.h>

//Creates a new post. A new empty comment_list is also created
post * new_post(int ts, long post_id, long user_id)
{
	post * ret_ref = malloc(sizeof(post));
	ret_ref->ts = ts;
	ret_ref->post_id = post_id;
	ret_ref->user_id = user_id;
	ret_ref->comment_list = new_comment_list();
	if(ret_ref->comment_list==NULL)
	{
		print_warning("Cannot malloc comment_list");
		return NULL;
	}
	print_info("Returning a new post with ts=%d, post_id=%ld, user_id=%ld", ret_ref->ts, ret_ref->post_id, ret_ref->user_id);
	return ret_ref;
}

//destroys the post and the comment_list associated to it
void del_post( post * post_ref)
{
	del_comment_list(post_ref->comment_list);
	free(post_ref);
}

//print to stdout the post instance
void print_post(post * post_ref)
{
	print_msg("POST","ts=%d, post_id=%ld, user_id=%ld, score=%d, comment_list=@%d", post_ref->ts, post_ref->post_id, post_ref->user_id, post_ref->score, post_ref->comment_list);
}

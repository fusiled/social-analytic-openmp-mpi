#include "post.h"

#include "comment_list.h"
#include "debug_utils.h"

#include <stdlib.h>

//Creates a new post. A new empty comment_list is also created
post * new_post(int ts, long post_id, long user_id)
{
	post * ret_ref = malloc(sizeof(post));
	if(ret_ref==NULL)
	{
		print_error("Cannot malloc a new post");
	}
	ret_ref->ts = ts;
	ret_ref->post_id = post_id;
	ret_ref->user_id = user_id;
	return ret_ref;
}

//destroys the post and the comment_list associated to it
void del_post( post * post_ref)
{
	free(post_ref);
}

//print to stdout the post instance
void print_post(post * post_ref)
{
	print_msg("POST","@%p,  ts=%d, post_id=%ld, user_id=%ld",post_ref, post_ref->ts, post_ref->post_id, post_ref->user_id);
}


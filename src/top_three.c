#include "top_three.h"


#include "debug_utils.h"

#include <stdlib.h>
#include <string.h>


top_three * new_top_three(int ts, long p_post_id[TOP_NUMBER], 
		long p_user_id[TOP_NUMBER], int p_n_commenters[TOP_NUMBER])
{
	top_three * ret_ref = malloc(sizeof(top_three));
	if(ret_ref==NULL)
	{
		print_error("Cannot malloc top_three");
		return NULL;
	}
	ret_ref->ts = ts;
	memcpy(ret_ref->post_id, p_post_id, sizeof(long)*TOP_NUMBER);
	memcpy(ret_ref->user_id, p_post_id, sizeof(long)*TOP_NUMBER);
	memcpy(ret_ref->n_commenters, p_n_commenters, sizeof(int)*TOP_NUMBER);
	return ret_ref;
}



void del_top_three(top_three * tt)
{
	free(tt);
}
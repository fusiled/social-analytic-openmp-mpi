#include "top_three.h"


#include "debug_utils.h"
#include "utils.h"

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
	print_info("ts: %d", ts);
	memcpy(ret_ref->post_id, p_post_id, sizeof(long)*TOP_NUMBER);
	memcpy(ret_ref->user_id, p_user_id, sizeof(long)*TOP_NUMBER);
	memcpy(ret_ref->n_commenters, p_n_commenters, sizeof(int)*TOP_NUMBER);
	return ret_ref;
}



void del_top_three(top_three * tt)
{
	free(tt);
}

//itoa and itol are in utils.h
void print_top_three(top_three * tt)
{
	char output[1024];
	char buf[256];
	output[0]='\0';
	itoa(tt->ts, buf);
	strcat(output,buf);
	for(int i=0; i<TOP_NUMBER; i++)
	{
		itoa(i+1,buf);
		strcat(output," [");
		strcat(output,buf);
		strcat(output,"] post_id: ");
		ltoa(tt->post_id[i],buf);
		strcat(output,buf);
		strcat(output," user_id: ");
		ltoa(tt->user_id[i],buf);
		strcat(output,buf);
		strcat(output, " n_commenters: ");
		ltoa(tt->n_commenters[i],buf);
		strcat(output,buf);
	}
	print_msg("TOP 3", output);
}
#include "top_three.h"


#include "debug_utils.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>


top_three * new_top_three(int ts, long p_post_id[TOP_NUMBER], 
		long p_user_id[TOP_NUMBER],int p_post_score[TOP_NUMBER], int p_n_commenters[TOP_NUMBER])
{
	top_three * ret_ref = malloc(sizeof(top_three));
	if(ret_ref==NULL)
	{
		print_error("Cannot malloc top_three");
		return NULL;
	}
	ret_ref->ts = ts;
	memcpy(ret_ref->post_id, p_post_id, sizeof(long)*TOP_NUMBER);
	memcpy(ret_ref->user_id, p_user_id, sizeof(long)*TOP_NUMBER);
	memcpy(ret_ref->post_score, p_post_score, sizeof(int)*TOP_NUMBER);
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
	if(tt==NULL)
	{
		print_warning("Passed NULL to print_top_three");
		return;
	}
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
		strcat(output, " post_score: ");
		itoa(tt->post_score[i],buf);
		strcat(output,buf);
		strcat(output, " n_commenters: ");
		itoa(tt->n_commenters[i],buf);
		strcat(output,buf);
	}
	print_msg("TOP 3", output);
}

char * to_string_tuple_top_three(top_three * tt)
{
	if(tt==NULL)
	{
		print_warning("Passed NULL to print_top_three. returning empty string");
		char * ret_empty = malloc(sizeof(char));
		ret_empty[0]='\0';
		return ret_empty;
	}
	char * output = malloc(sizeof(char)*1024);
	if(output==NULL)
	{
		print_error("Cannot malloc array for to_string_tuple_top_three");
	}
	char buf[256];
	output[0]='\0';
	itoa(tt->ts, buf);
	strcat(output,buf);
	for(int i=0; i<TOP_NUMBER; i++)
	{
		strcat(output,",");
		ltoa(tt->post_id[i],buf);
		strcat(output,buf);
		strcat(output,",");
		ltoa(tt->user_id[i],buf);
		strcat(output,buf);
		strcat(output, ",");
		itoa(tt->post_score[i],buf);
		strcat(output,buf);
		strcat(output, ",");
		itoa(tt->n_commenters[i],buf);
		strcat(output,buf);
	}
	return output;
}


int compare_top_three_score(top_three * first, top_three * second)
{
	if(first==NULL)
	{
		if(second==NULL)
		{
			print_warning("Passed a double NULL to compare_top_three_score. Returning 0");
			return 0;
		}
		else
		{
			print_warning("Passed first argument of compare_top_three_score set to NULL, returning -1");
			return -1;
		}
	}
	else
	{
		if(second == NULL )
		{
			print_warning("Passed second argument of compare_top_three_score set to NULL; returning 1");
			return 1;
		}
		else
		{
		for(int i=0; i<TOP_NUMBER; i++)
		{
			int res_tmp = first->post_score[i] - second->post_score[i];
			if(res_tmp!=0)
			{
				return res_tmp;
			}
		}
		return 0;
		}
	}
}

int compare_top_three(top_three * first, top_three * second)
{
	if(first==NULL)
	{
		if(second==NULL)
		{
			print_warning("Passed a double NULL to compare_top_three. Returning 0");
			return 0;
		}
		else
		{
			print_warning("Passed first argument of compare_top_three set to NULL, returning -1");
			return -1;
		}
	}
	else
	{
		if(second == NULL )
		{
			print_warning("Passed second argument of compare_top_three set to NULL; returning 1");
			return 1;
		}
		else
		{
			int ts_cmp = first->ts - second->ts;
			if(ts_cmp!=0)
			{
				return ts_cmp;
			}
			int cmp_score = compare_top_three_score(first, second);
			if(cmp_score!=0)
			{
				return cmp_score;
			}
			for(int i=0; i<TOP_NUMBER; i++)
			{
				long pid = first->post_id[i] - second->post_id[i];
				if(pid!=0)
				{
					return pid;
				}
				long uid = first->user_id[i] - second->user_id[i];
				if(uid!=0)
				{
					return uid;
				}
				int nc = first->n_commenters[i] - second->n_commenters[i];
				if(nc!=0)
				{
					return nc;
				}
			}
			return 0;
		}
	}
}


int compare_top_three_without_timestamp(top_three * first, top_three * second)
{
	if(first==NULL)
	{
		if(second==NULL)
		{
			print_warning("Passed a double NULL to compare_top_three. Returning 0");
			return 0;
		}
		else
		{
			print_warning("Passed first argument of compare_top_three set to NULL, returning -1");
			return -1;
		}
	}
	else
	{
		if(second == NULL )
		{
			print_warning("Passed second argument of compare_top_three set to NULL; returning 1");
			return 1;
		}
		else
		{
			int cmp_score = compare_top_three_score(first, second);
			if(cmp_score!=0)
			{
				return cmp_score;
			}
			for(int i=0; i<TOP_NUMBER; i++)
			{
				long pid = first->post_id[i] - second->post_id[i];
				if(pid!=0)
				{
					return pid;
				}
				long uid = first->user_id[i] - second->user_id[i];
				if(uid!=0)
				{
					return uid;
				}
				int nc = first->n_commenters[i] - second->n_commenters[i];
				if(nc!=0)
				{
					return nc;
				}
			}
			return 0;
		}
	}

}
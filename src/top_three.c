#include "top_three.h"


#include "debug_utils.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>


#define UNDEF_SIGN "-"


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
	print_msg("TOP 3", "%s", output);
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
		if(tt->post_id[i]==-1)
		{
			strcat(output,UNDEF_SIGN);
		}
		else
		{
			ltoa(tt->post_id[i],buf);
			strcat(output,buf);
		}
		strcat(output,",");
		if(tt->user_id[i]==-1)
		{
			strcat(output,UNDEF_SIGN);
		}
		else
		{
			ltoa(tt->user_id[i],buf);
			strcat(output,buf);
		}
		strcat(output, ",");
		if(tt->post_score[i]==-1)
		{
			strcat(output,UNDEF_SIGN);
		}
		else
		{
			itoa(tt->post_score[i],buf);
			strcat(output,buf);
		}
		strcat(output, ",");
		if(tt->post_score[i]==-1)
		{
			strcat(output,UNDEF_SIGN);
		}
		else
		{
			itoa(tt->n_commenters[i],buf);
			strcat(output,buf);
		}
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


char check_quit_combine(int * counter_ar, int size, int out_counter)
{
	char quit=1;
	if(out_counter>=TOP_NUMBER)
	{
		return quit;
	}
	for(int i=0; i<size; i++)
	{
		if(counter_ar[i]<TOP_NUMBER)
		{
			quit=0;
		}
	}
	return quit;
}



top_three * combine_top_three(top_three ** tt_ar, int size)
{
	//check that ts is the same, otherwise NULL is returned;
	if(tt_ar==NULL || size==0)
	{
		return NULL;
	}
	int ts = tt_ar[0]->ts;
	for(int i=1; i<size;i++)
	{
		if(tt_ar[i]->ts!=ts)
		{
			return NULL;
		}
	}
	int * counter_ar = calloc(sizeof(int),size);
	long post_id[TOP_NUMBER];
	long user_id[TOP_NUMBER];
	int post_score[TOP_NUMBER];
	int n_commenters_ar[TOP_NUMBER];
	int out_counter = 0;
	while(check_quit_combine(counter_ar,size, out_counter)==0)
	{
		//get element of the top_three with the biggest score
		int selected_tt=-1;
		int selected_el=-1;
		int score = -1;
		int n_commenters = -1;
		for(int i=0; i<size;i++)
		{
			int counter = counter_ar[i];
			if(score < tt_ar[i]->post_score[counter] || 
				(score == tt_ar[i]->post_score[counter] && n_commenters < tt_ar[i]->n_commenters[counter]) )
			{
				score = tt_ar[i]->post_score[counter];
				selected_tt = i;
				selected_el = counter;
				n_commenters = tt_ar[i]->n_commenters[counter];
			}
		}
		//push it into the arrays
		post_id[out_counter]=tt_ar[selected_tt]->post_id[selected_el];
		user_id[out_counter]=tt_ar[selected_tt]->user_id[selected_el];
		post_score[out_counter]=tt_ar[selected_tt]->post_score[selected_el];
		n_commenters_ar[out_counter]=tt_ar[selected_tt]->n_commenters[selected_el];

		//increase counters
		counter_ar[selected_tt] = counter_ar[selected_tt]+1;
		out_counter++;
	}

	top_three * out_ref = new_top_three(ts,post_id,user_id,post_score,n_commenters_ar);
	free(counter_ar);
	return out_ref;
}
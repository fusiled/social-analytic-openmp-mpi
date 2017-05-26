#include "comment_list.h"


#include <stdlib.h>
#include <string.h>

#include "debug_utils.h"

//constructor for a new list_entry with the passed comment
list_entry * new_list_entry(comment * com);


//destroy the passed list_entry.
void del_list_entry(list_entry * l_entry);

//create an empty comment list
comment_list * new_comment_list()
{
	comment_list * ret_com_list = malloc(sizeof(comment_list));
	if(ret_com_list==NULL)
	{
		return NULL;
	}
	ret_com_list->head = NULL;
	ret_com_list->size = 0;
	return ret_com_list;
}


//add a new comment com to com_list at the head of the list. return the new size of the list
int add_to_comment_list(comment_list * com_list, comment * com)
{
	list_entry * new_head = new_list_entry(com); 
	if (new_head==NULL)
	{
		return -1;
	}
	new_head->next = com_list->head;
	com_list->head = new_head;
	com_list->size = com_list->size + 1;
	return com_list->size;
}

//this method also destroys all the elements contained in the list
void del_comment_list(comment_list * com_list)
{
	if(com_list==NULL)
	{
		return;
	}
	list_entry * cur = com_list->head;
	while(cur!=NULL)
	{
		list_entry * buf = cur->next;
		del_list_entry(cur);
		cur = buf;
	}
	free(com_list);
}

//constructor for a new list_entry with the passed comment
list_entry * new_list_entry(comment * com)
{
	list_entry * ret_ref = malloc(sizeof(list_entry));
	if(ret_ref==NULL)
	{
		return NULL;
	}
	ret_ref->comment = com;
	ret_ref->next = NULL;
	return  ret_ref;
}

//destroy the passed list_entry.
void del_list_entry(list_entry * l_entry)
{
	free(l_entry->comment);
	free(l_entry);
}

/**
* \brief produces out_ts and out_user_id from the comment list cl
* \param[in] cl reference to the comment list that will be processes
* \param[out] out_ts reference to the array where timestamps will be saved
* \param[out] out_user_id reference to the array where user_id of the comments 
*		will be saved
* \param[out] reference to where the dimension of the 2 produced array will be 
*		saved
**/
void build_arrays_from_comment_list(comment_list * cl, int ** out_ts,
		long ** out_user_id, int * out_size)
{
	if(cl==NULL)
	{
		*out_ts=NULL;
		*out_user_id=NULL;
		*out_size=0;
		//print_info("Passed a NULL pointer to build_array_from comment_list_function");
		return;
	}
	int cl_size = cl->size;
	int * ts_ref = calloc(sizeof(int), cl_size);
	long * user_id_ref = calloc(sizeof(long), cl_size);
	if(ts_ref == NULL)
	{
		print_error("Cannot alloc ts_ref");
		return;
	}
	if(user_id_ref == NULL)
	{
		print_error("Cannot alloc user_id_ref");
		return;
	}
	int counter = 0;
	list_entry * cur = cl->head;
	//iterate over the list and save
	while(cur!=NULL)
	{
		*(ts_ref+counter) = cur->comment->ts;
		user_id_ref[counter]= cur->comment->user_id;
		counter++;
		cur = cur->next;
	}
	*out_size=cl_size;
	*out_ts=ts_ref;
	*out_user_id=user_id_ref;
	//print_fine("Produced arrays from comment list of size %d", *out_size);
}


void print_comment_list(comment_list * cl)
{
	//just build the string on the variable buffer and print it.
	char buffer[256];
	char ltoa_buf[64];
	buffer[0]='|';
	buffer[1]='\0';
	list_entry * cur = cl->head;
	while(cur!=NULL)
	{
		comment * comm = cur->comment;
		snprintf(ltoa_buf,sizeof(ltoa_buf),"%ld",comm->comm_id);
		strcat(buffer,ltoa_buf);
		strcat(buffer, "->");
		cur = cur->next;
	}
	strcat(buffer, "|");
	print_msg("COMMENT LIST", buffer);
}
#include "comment_list.h"


#include <stdlib.h>


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
	list_entry * cur = com_list->head;
	while(cur!=NULL)
	{
		list_entry * buf = cur->next;
		del_list_entry(cur);
		cur = buf;
	}
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



void build_arrays_from_comment_list(comment_list * cl, int * out_ts, long * out_user_id, int * out_size)
{
	int cl_size = cl->size;
	out_ts = calloc(sizeof(int), cl_size);
	out_user_id = calloc(sizeof(long), cl_size);
	int counter = 0;
	list_entry * cur = cl->head;
	while(cur!=NULL)
	{
		out_ts[counter]= (cur->comment)->ts;
		out_user_id[counter]= (cur->comment)->user_id;
		counter++;
		cur = cur->next;
	}
	*out_size=cl_size;
}
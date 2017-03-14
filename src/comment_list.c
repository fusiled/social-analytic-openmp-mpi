#include "comment_list.h"


#include <stdlib.h>


//constructor for a new list_entry with the passed comment
list_entry * new_list_entry(comment * com);


//destroy the passed list_entry.
void del_list_entry(list_entry * l_entry);


//create an empty comment list
//this is a simple list creator
comment_list * new_comment_list()
{
	comment_list * ret_com_list = malloc(sizeof(comment_list));
	if(ret_com_list==NULL)
	{
		return NULL;
	}
	ret_com_list->head = NULL;
	return ret_com_list;
}


//add a new comment com to com_list at the head of the list. Once you pass a 
//comment, then the comment_list is responsible to delete it.
int add_to_comment_list(comment_list * com_list, comment * com)
{
	list_entry * new_head = new_list_entry(com); 
	if (new_head==NULL)
	{
		return -1;
	}
	new_head->next = com_list->head;
	com_list->head = new_head;
	return 1;
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
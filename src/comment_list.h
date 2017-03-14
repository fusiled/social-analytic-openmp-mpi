#ifndef COMMENT_LIST_H
#define COMMENT_LIST_H


#include "comment.h"

/**
* Header for the comment lists. It is a simple linked list. comments are wrapped in list_entry elements.
* elements are inserted at the head.
* NOTE: list_entry should never be created by the user.
**/

typedef struct list_entry_struct
{
	comment * comment;
	struct list_entry_struct * next;
} list_entry;

typedef struct comment_list_struct
{
	list_entry * head;

} comment_list;

//create an empty comment list
comment_list * new_comment_list();

//add a new comment com to com_list at the head of the list. Once you pass a 
//comment, then the comment_list is responsible to delete it.
int add_to_comment_list(comment_list * com_list, comment * com);

//this method also destroys all the elements contained in the list
void del_comment_list(comment_list * com_ch);

#endif
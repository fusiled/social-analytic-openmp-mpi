#ifndef COMMENT_LIST_H
#define COMMENT_LIST_H


#include "comment.h"

#include <stdio.h>

/**
* Header for the comment lists. It is a simple linked list. comments are wrapped in list_entry elements.
* elements are inserted at the head. When a comment is inside a comment_list then the list is responsible
* to delete it. del_comment_list should do all the dirty job of deletions.
* NOTE: list_entry should never be created by the user. just use add_to_comment_list
**/

typedef struct list_entry
{
	comment * comment;
	struct list_entry * next;
} list_entry;

typedef struct comment_list
{
	list_entry * head;
	int size;

} comment_list;

//create an empty comment list
comment_list * new_comment_list();

//add a new comment com to com_list at the head of the list. Once you pass a 
//comment, then the comment_list is responsible to delete it.
int add_to_comment_list(comment_list * com_list, comment * com);

//this method also destroys all the elements contained in the list
void del_comment_list(comment_list * com_ch);


/**
* \brief produces out_ts and out_user_id from the comment list cl
* \param[in] cl reference to the comment list that will be processes
* \param[out] out_ts reference to the array where timestamps will be saved
* \param[out] out_user_id reference to the array where user_id of the comments 
*		will be saved
* \param[out] reference to where the dimension of the 2 produced array will be 
*		saved
**/
void build_arrays_from_comment_list(comment_list * cl, int ** out_ts, long ** out_user_id, int * out_size);

/** 
* \brief print a comment list at the standard out
* \param[in] cl the comment list to print
**/
void print_comment_list(comment_list * cl);


#endif
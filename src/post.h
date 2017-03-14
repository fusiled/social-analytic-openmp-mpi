
#ifndef POST_H
#define POST_H

#include "comment_list.h"

/**
* Header for the post struct and all the function utilities associated to it
**/

typedef struct post_struct
{
	int ts;
	long post_id;
	long user_id;
	int score;
	comment_list * comment_list;
} post;

//Creates a new post. A new empty comment_list is also created
post * new_post(int ts, long post_id, long user_id);

//destroys the post and the comment_list associated to it
void del_post( post * post_ref);

//print to stdout the post instance
void print_post( post * post);

#endif


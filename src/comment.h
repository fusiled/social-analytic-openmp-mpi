#ifndef COMMENT_H
#define COMMENT_H

#include "reply_type.h"

typedef struct comment_struct
{
	int ts;
	long comm_id;
	long user_id;
	long reply_id;
	reply_type repl_ty;
}comment;


//create a new comment with the given timestamp and user_id (all we need)
comment * new_comment(int ts,  long comm_id, long user_id, long reply_id, reply_type repl_ty);


//destroy the comment
void del_comment(comment * com);

void print_comment(comment * com);

#endif
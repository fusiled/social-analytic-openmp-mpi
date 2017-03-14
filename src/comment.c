#include "comment.h"

#include "debug_utils.h"
#include "reply_type.h"

#include <stdlib.h>

extern const reply_type COMMENT_REPLY_TYPE;
extern const reply_type POST_REPLY_TYPE;

//create a new comment
comment * new_comment(int ts, long comm_id, long user_id, long reply_id, reply_type repl_ty)
{
	if(reply_id==0)
	{
		print_error("passed a reply_id equal to zero to comment constructor");
		return NULL;
	}
	if(comm_id==0)
	{
		print_error("passed a comm_id equal to zero to comment constructor");
		return NULL;
	}
	if(user_id==0)
	{
		print_error("passed a user_id equal to zero to comment constructor");
		return NULL;
	}
	comment * ret_ref = malloc(sizeof(comment));
	if(ret_ref==NULL)
	{
		return NULL;
	}
	ret_ref->ts = ts;
	ret_ref->comm_id = comm_id;
	ret_ref->user_id = user_id;
	ret_ref->reply_id = reply_id;
	ret_ref->repl_ty = repl_ty;
}

//desotry the comment
void del_comment(comment * com)
{
	free(com);
}

void print_comment(comment * com)
{
	print_msg("COMM","ts=%d, comm_id=%ld, user_id=%ld, reply_id=%ld, reply_type=%d", com->ts, com->comm_id,com->user_id, com->reply_id, com->repl_ty);
}
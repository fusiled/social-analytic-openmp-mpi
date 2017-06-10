#ifndef COMMENT_H
#define COMMENT_H

#include "reply_type.h"


//struct that wraps the useful informations about a comment in the preprocessing phase
typedef struct comment
{
    int ts;
    long comm_id;
    long user_id;
    long reply_id;
    reply_type repl_ty;
} comment;

//create a new comment with the given timestamp, comment_id, user_id, id of the post/comment
//that it has been replied and the reply type (that can be POST_REPLY_TYPE or COMMENT_REPLY_TYPE)
//If invalid argumants are passed of malloc fails then NULL is returned
comment * new_comment(int ts,  long comm_id, long user_id, long reply_id, reply_type repl_ty);

//destroy the comment
void del_comment(comment * com);

//print to stdout the comment
void print_comment(comment * com);

#endif
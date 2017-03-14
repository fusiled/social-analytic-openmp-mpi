
#ifndef PARSER_H
#define PARSER_H


#include "post.h"
#include "comment.h"

#include <stdio.h>



post * parse_post(FILE * post_fp, int n_lines, int * read_lines);


comment * parse_comment(FILE * comment_fp, int n_lines, int * read_lines);


#endif



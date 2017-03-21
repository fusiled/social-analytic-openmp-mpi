#include "parser.h"
#include "post.h"
#include "comment.h"
#include "debug_utils.h"
#include "utils.h"
#include "reply_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern const reply_type COMMENT_REPLY_TYPE;
extern const reply_type POST_REPLY_TYPE;

const int POST_TIMESTAMP = 0;
const int POST_ID = 1;
const int POST_USER_ID = 2;

const int COMMENT_TIMESTAMP = 0;
const int COMMENT_ID = 1;
const int COMMENT_USER_ID = 2;
//comment: is a string containing the actual comment
//user: is a string containing the actual user name
const int COMMENT_COMMENT_REPLIED = 5;
const int COMMENT_POST_REPLIED = 6;

const char * POST_DELIMITERS = "|";
const char * COMMENT_DELIMITERS = "|";
const char * TIME_FORMAT_STRING = "%Y-%m-%dT%H:%M:%S";
const int POST_INPUT_ENTRIES = 5;
const int COMMENT_INPUT_ENTRIES = 7;

/**
* returns an array of strings that contains the chunks of line splitted by delim.
* Two consecutive delims produces an empty string. This function will surely return
* an array of char * that is dim big. It is your responsibility to free the
* returned array and the pointed elements
**/
char ** parse_line(char * line, const char * delim, int dim)
{
	char ** ret_ref = calloc(sizeof(char *), dim);
    char *p, *var2, *var3;
    int ret_iterator=0;
 	var2=strdup(line);   // allocates enough space for var1 and copies the contents
    var3=var2;           // save off var2, since strsep changes it
    while ((p = strsep(&var2,delim)) != NULL) {   // p contains the token
    	ret_ref[ret_iterator]=malloc(sizeof(char)*(strlen(p)+1));
    	memcpy(ret_ref[ret_iterator],p, sizeof(char)*(strlen(p)+1) );
    	ret_iterator++;
    }
    free(var3);          // var2 is now NULL, so use var3 instead
    return ret_ref;
}

/**
* Get the array produced by parse_line(line, POST_DELIMITERS) and produce a new_post. The passed line
* must be an entry of the posts.dat file
**/
post * process_post_line(char * line)
{
	int ts;
	long post_id, user_id;
	char ** buf = parse_line(line,POST_DELIMITERS, POST_INPUT_ENTRIES);
	ts = get_int_time(buf[POST_TIMESTAMP], TIME_FORMAT_STRING);
	//atoll gets a string and produces a long long unsigned int representation of it.
    post_id = atol(buf[POST_ID]);
    user_id = atol(buf[POST_USER_ID]);
	//free the buf array (see utils.h)
	del_double_ref_array( (void **)buf, POST_INPUT_ENTRIES);
	return new_post(ts, post_id, user_id);
}


/**
* Get the array produced by parse_line(line, COMMENT_DELIMITERS) and produce a new_comment. The passed line
* must be an entry of the comments.dat file
**/
comment * process_comment_line(char * line)
{
    int ts;
    long comm_replied, user_id, reply_id, post_replied, comment_id;
    reply_type repl_ty;
    char ** buf = parse_line(line,COMMENT_DELIMITERS, COMMENT_INPUT_ENTRIES);
    ts = get_int_time(buf[COMMENT_TIMESTAMP], TIME_FORMAT_STRING);
    //atol gets a string and produces a long int representation of it.
    user_id = atol(buf[COMMENT_USER_ID]);
    comment_id = atol(buf[COMMENT_ID]);
    comm_replied = atol(buf[COMMENT_COMMENT_REPLIED]);
    post_replied = atol(buf[COMMENT_POST_REPLIED]);
    //free the buf array (see utils.h)
    del_double_ref_array( (void **)buf, POST_INPUT_ENTRIES);
    if(comm_replied!=0)
    {
        reply_id = comm_replied;
        repl_ty = COMMENT_REPLY_TYPE;
    }
    else
    if(post_replied!=0){
        reply_id = post_replied;
        repl_ty = POST_REPLY_TYPE;
    }
    else
    {
        return NULL;    
    }
    return new_comment(ts, comment_id, user_id, reply_id, repl_ty);
}


/**
*  Parse n_lines starting from post_fp. It returns an array of posts which will be read_lines big.
* You can know the number of read_lines through the pointer read_lines.
**/
post * parse_post(FILE * post_fp, int n_lines, int * read_lines)
{
	if(n_lines<=0 || post_fp==NULL)
	{
		return NULL;
	}
    if(feof(post_fp)){
        print_warning("reached end of file... returning null");
        return NULL;
    }
	//the number of read lines from this function. This variable
	//will be returned through the pointer read_lines
	int return_read_lines=0;
	//allocate some space to save posts. This will be trimmed if
	//at the end of the while return_read_lines < n_lines
	post * post_array = calloc(sizeof(post), n_lines);
	if(post_array==NULL)
	{
		*read_lines=0;
		return NULL;
	}
	//file related stuff
    size_t len = 0;
    ssize_t read;
    //buffer for the current line.
	char * line = NULL;
    while ( (return_read_lines<n_lines) && ((read = getline(&line, &len, post_fp)) != -1) )
    {
    	//print_fine("read line %s", line);
    	post * post_buf=process_post_line(line);
    	if(post_buf==NULL)
    	{
    		print_warning("Error in process_post_line");
    		*read_lines=return_read_lines;
    		break;
    	}
    	else
    	{
    		memcpy(post_array+return_read_lines,post_buf, sizeof(post));
    	}
    	return_read_lines++;
    }
    if (line)
    {
        free(line);
    }
    //trim the post array. We don't want trash at the end of it. We will copy it into a new one of the correct dimension
    post * trimmed_post_array = calloc(sizeof(post), return_read_lines);
    if(trimmed_post_array==NULL)
    {
        print_error("Error on malloc of trimmed_post_array");
    	free(post_array);
    	return NULL;
    }
    memcpy(trimmed_post_array, post_array, sizeof(post)*return_read_lines);
    free(post_array);
    *read_lines=return_read_lines;
    return trimmed_post_array;
}



//it is similar to parse_post. The logic is exactly the same. FIXME merge code between parse_comment and parse_post
comment * parse_comment(FILE * comm_fp, int n_lines, int * read_lines)
{
    if(n_lines<=0 || comm_fp==NULL)
    {
        return NULL;
    }
    if(feof(comm_fp)){
        print_warning("reached end of file... returning null");
        return NULL;
    }
    //the number of read lines from this function. This variable
    //will be returned through the pointer read_lines
    int return_read_lines=0;
    //allocate some space to save posts. This will be trimmed if
    //at the end of the while return_read_lines < n_lines
    comment * comm_array = calloc(sizeof(comment), n_lines);
    if(comm_array==NULL)
    {
        *read_lines=0;
        return NULL;
    }
    //file related stuff
    size_t len = 0;
    ssize_t read;
    //buffer for the current line.
    char * line = NULL;
    while (  (return_read_lines<n_lines) && ((read = getline(&line, &len, comm_fp)) != -1) )
    {
        //print_fine("read line %s", line);
        comment * comm_buf=process_comment_line(line);
        if(comm_buf==NULL)
        {
            print_warning("Error in process_post_line");
            *read_lines=return_read_lines;
            break;
        }
        else
        {
            memcpy(comm_array+return_read_lines,comm_buf, sizeof(comment));
        }
        return_read_lines++;
    }
    if (line)
    {
        free(line);
    }
    //trim the comment array. We don't want trash at the end of it. We will copy it into a new one of the correct dimension
    comment * trimmed_comm_array = calloc(sizeof(comment), return_read_lines);
    if(trimmed_comm_array==NULL)
    {
        free(comm_array);
        return NULL;
    }
    memcpy(trimmed_comm_array, comm_array, sizeof(comment)*return_read_lines);
    free(comm_array);
    *read_lines=return_read_lines;
    return trimmed_comm_array;
}
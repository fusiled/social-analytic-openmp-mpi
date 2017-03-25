
#ifndef TOP_THREE_H
#define TOP_THREE_H


#define TOP_NUMBER 3

/**
* ts: is the timestamp of the tuple event that triggers a change in the top-3
*	scoring active posts appearing in the rest of the tuple
* topX_post_id: is the unique id of the top-X post
* topX_post_user: the user author of top-X post
* topX_post_commenters: the number of commenters (excluding the post author) for the top-X post
*/

typedef struct top_three
{
	int ts;
	long post_id[TOP_NUMBER];
	long user_id[TOP_NUMBER];
	int n_commenters[TOP_NUMBER];
} top_three;


top_three * new_top_three(int ts, long p_post_id[TOP_NUMBER], 
	long p_user_id[TOP_NUMBER], int p_n_commenters[TOP_NUMBER]);


void del_top_three(top_three * tt);


void print_top_three(top_three * tt);


#endif
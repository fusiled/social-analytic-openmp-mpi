
#include "parse_events.h"
#include "top_three.h"
#include "event_list.h"

#include <stdio.h>
#include <stdlib.h>


#define SIZE 200

int main(int argc, char const *argv[])
{
	valued_event * var = malloc(sizeof(valued_event)*SIZE);
	for(int i=0; i< SIZE; i++)
	{
		valued_event * buf = new_valued_event(i,i%20,i%4,(i*121)%93,i);
		var[i]=*buf;
		clear_valued_event(buf);
	}
	int out_size;
	top_three ** out_tt = parse_events(var,SIZE,&out_size);
	printf("out_size: %d\n", out_size);
	for(int i=0; i<out_size;i++)
	{
		print_top_three(out_tt[i]);
		del_top_three(out_tt[i]);
	}
	free(out_tt);
	free(var);
	return 0;
}
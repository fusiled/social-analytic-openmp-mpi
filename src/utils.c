#include "utils.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>


#include "debug_utils.h"

/**
* Header for utility functions
*
**/

void del_double_ref_array(void ** array, int size)
{
	//freeing elements of array and also array
	for(int i=0; i<size; i++)
	{
		free(array[i]);
	}
	free(array);
}

int get_int_time(char * time_string, const char * time_format_string)
{
	struct tm tm;
	strptime(time_string, time_format_string, &tm);
	time_t t = mktime(&tm);
	return t;
}


void print_int_array(int * array, int size)
{
	char buf[256];
	char small_buf[64];
	buf[0]='\0';
	for(int i=0; i<size; i++)
	{
		snprintf(small_buf,sizeof(small_buf),"%d",array[i]);
		strcat(buf,small_buf);
		strcat(buf, " ");
	}
	print_msg("INT ARRAY", buf);
}
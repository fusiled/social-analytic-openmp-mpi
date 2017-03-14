#include "utils.h"

#include <stdlib.h>
#include <time.h>


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
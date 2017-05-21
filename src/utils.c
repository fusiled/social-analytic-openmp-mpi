#include "utils.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>


#include "debug_utils.h"


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
	struct tm t;
    char * buf = strtok(time_string,"-T:.");
    t.tm_year = atoi(buf)-1900;
    buf = strtok(NULL,"-T:.");
    t.tm_mon = atoi(buf)-1; 
    buf = strtok(NULL,"-T:.");  
    t.tm_mday = atoi(buf);
    buf = strtok(NULL,"-T:.");
    t.tm_hour = atoi(buf);
    buf = strtok(NULL,"-T:.");
    t.tm_min = atoi(buf);
    buf = strtok(NULL,"-T:.");
    t.tm_sec = atoi(buf);
    t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
	time_t sec_time = mktime(&t);
	return (int)sec_time;
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



//just wrap snprintf
void itoa(int number, char * buffer)
{
	sprintf(buffer,"%d",number);
}

//just wrap snprintf
void ltoa(long number, char * buffer)
{
	sprintf(buffer,"%ld",number);
}

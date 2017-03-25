#ifndef UTILS_H
#define UTILS_H


void del_double_ref_array( void ** array, int size);


//pass a string that represent time and a string that represents the time format
//of the time_string. The function will return the seconds passed from the UNIX
//reference date
int get_int_time(char * time_string, const char * time_format_string);


//Print to stdout an array of integers of size big
void print_int_array(int * array, int size);

//just wrap snprintf
void itoa(int number, char * buffer);

//just wrap snprintf
void ltoa(long number, char * buffer);

#endif
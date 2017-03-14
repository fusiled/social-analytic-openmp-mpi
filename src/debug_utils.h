#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H


/**
* Print to stdout considering the DEBUG_LEVEL. See the implementation for furthet details
*
**/

void set_debug_level(int new_level);

int get_debug_level();

void print_msg(char * tag, char * msg, ...);

void print_warning(char * msg, ...);

void print_info(char * msg, ...);

void print_fine(char * msg, ...);

void print_error(char * msg, ...);


#endif
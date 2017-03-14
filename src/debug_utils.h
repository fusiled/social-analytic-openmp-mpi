#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H


/**
* Print to stdout considering the DEBUG_LEVEL. It's just a simple wrapper of printf
* NOTE: print_msg will ignore DEBUG_LEVEL, so for generic printing you can use it.
* just remember to pass a proper tag. For more details (level hierarchy) see debug_utils.c
*
* DEBUG_LEVEL is set in debug_utils.c
**/

void set_debug_level(int new_level);

int get_debug_level();

void print_msg(char * tag, char * msg, ...);

void print_warning(char * msg, ...);

void print_info(char * msg, ...);

void print_fine(char * msg, ...);

void print_error(char * msg, ...);


#endif
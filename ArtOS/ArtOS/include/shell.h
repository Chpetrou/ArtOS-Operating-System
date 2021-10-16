#ifndef __SHELL_H_
#define __SHELL_H_

#include <vector.h>
#include <terminal.h>

/* Shell commands structure */
struct shell_command {
    char cmname[25];
    int (*function) ();
    struct systemLang *description;
};

struct shell_command cmdArray[10];
int cmdArrayIndex;

//Basic shell commands
int add_shell_command(char * cmname, int (*function) (), struct systemLang * description);

int shell_main();

int calc_main();

int get_time_of_day();

int get_calc();

int shut_down();

#endif

#include <stdio.h>
#include <stdlib.h>

#include "help.h"

void help_cd() {
    printf("\nCOMMAND : CD\n\n");
    printf("USAGE : cd <path>\n\n");
    printf("DESCRIPTION : Goes to the home directory. If a path is specified, goes to the path\n\n");
    printf("OPTIONS :\n");
    printf("\t- PATH (optional): relative or absolute path to the directory you want to go\n\n");
}

void help_cat() {
    printf("\nCOMMAND : CAT\n\n");
    printf("USAGE : CAT <filepath>\n\n");
    printf("DESCRIPTION : Opens the file specified in read mode and prints the output\n\n");
    printf("OPTIONS :\n");
    printf("\t- FILEPATH (optional): relative or absolute path to the file you want to read\n\n");
}
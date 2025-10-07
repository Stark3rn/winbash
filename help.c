#include <stdio.h>
#include <stdlib.h>

#include "help.h"

void help_cd() {
    printf("\nCOMMAND : CD\n\n");
    printf("USAGE : cd [path]\n\n");
    printf("DESCRIPTION : Goes to the home directory. If a path is specified, goes to the path\n\n");
    printf("OPTIONS :\n");
    printf("\t- PATH (optional): relative or absolute path to the directory you want to go\n\n");
}

void help_cat() {
    printf("\nCOMMAND : CAT\n\n");
    printf("USAGE : cat <filepath>\n\n");
    printf("DESCRIPTION : Opens the file specified in read mode and prints the output\n\n");
    printf("OPTIONS :\n");
    printf("\t- FILEPATH : relative or absolute path to the file you want to read\n\n");
}

void help_echo() {
    printf("\nCOMMAND : ECHO\n\n");
    printf("USAGE : echo <message>\n\n");
    printf("DESCRIPTION : Prints your message ! Add '> <filepath>' to redirect output to a file.\n\n");
    printf("OPTIONS :\n");
    printf("\t- MESSAGE : message you want to print\n\n");
}

void help_exit() {
    printf("\nCOMMAND : EXIT\n\n");
    printf("USAGE : exit\n\n");
    printf("DESCRIPTION : Exits the shell.\n\n");
}

void help_help() {
    printf("\nCOMMAND : HELP\n\n");
    printf("USAGE : help [command]\n\n");
    printf("DESCRIPTION : Provides informations about the commands.\n\n");
    printf("OPTIONS :\n");
    printf("\t- COMMMAND (optional): command you want details on.\n\n");
}

void help_ls() {
    printf("\nCOMMAND : LS\n\n");
    printf("USAGE : ls [path] [options]\n\n");
    printf("DESCRIPTION : List the files contained in a given directory.\n\n");
    printf("OPTIONS :\n");
    printf("\t- path (optional): relative or absolute path of the folder you want to list. Current directory by default.\n");
    printf("\t- options (optional): additionnal options you want to use. The current options are:\n");
    printf("\t\t- '-h' : Show hidden files.\n\n");
}

void help_pwd() {
    printf("\nCOMMAND : PWD\n\n");
    printf("USAGE : pwd\n\n");
    printf("DESCRIPTION : Show the current directory's path.\n\n");
}
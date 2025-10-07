#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "help.h"

// WINDOWS INCLUDES
#ifdef _WIN32

#define OS "WINDOWS"
#include <windows.h>
#include <fcntl.h>
#include <io.h>

#endif

// UNIX INCLUDES
#ifdef __unix__

#define OS "UNIX"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>

#endif

#include "consts.h"


char cwd[MAX_PATH];
char *ENV_HOME;

// ==== BUILTIN COMMANDES ==== //


int shell_cd(char **args) {
#ifdef _WIN32
    if(!args[1]) {
        if (!SetCurrentDirectory(ENV_HOME)) {
            fprintf(stderr, "\033[31mERREUR : %s n'est pas un chemin valide.\033[0m\n", ENV_HOME);
        return EXIT_SHELL_ERROR;
    }
    } else if (!SetCurrentDirectory(args[1])) {
        fprintf(stderr, "\033[31mERREUR : %s n'est pas un chemin valide.\033[0m\n", args[1]);
        return EXIT_SHELL_ERROR;
    }
    return(EXIT_SHELL_SUCCESS);
# else
    if (!args[1]) {
        if (chdir(ENV_HOME) != 0) {
            perror("cd");
            return EXIT_SHELL_ERROR;
        }
    } else if (chdir(args[1]) != 0) {
        perror("cd");
        return EXIT_SHELL_ERROR;
    }
    return EXIT_SHELL_SUCCESS;
#endif
}

int shell_cat(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "\033[31mERREUR : Aucun fichier spécifié.\033[0m\n");
        return EXIT_SHELL_ERROR;
    }

    FILE *f = fopen(args[1], "r");
    if (!f) {
        fprintf(stderr, "\033[31mERREUR : Impossible d'ouvrir %s\033[0m\n", args[1]);
        return EXIT_SHELL_ERROR;
    }

    char line[SHELL_BUFFSIZE];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }

    fclose(f);
    return EXIT_SHELL_SUCCESS;
}

int shell_echo(char **args) {
    if(!args[1]) {
        fprintf(stderr,"\033[31mERREUR : il faut entrer des arguments.\033[0m\n");
        return(EXIT_SHELL_ERROR);
    }
    if(!args[2]) {
        printf("%s\n",args[1]);
        return(EXIT_SHELL_SUCCESS);
    }
    if(strcmp(">",args[2])==0) {
        FILE *f=fopen(args[3],"w");
        if(!f) {
            fprintf(stderr,"\033[31mERREUR : impossible d'ouvrir %s\033[0m\n",args[3]);
            return EXIT_SHELL_ERROR;
        }
        fprintf(f,"%s",args[1]);
        fclose(f);
        return(EXIT_SHELL_SUCCESS);
    }
    if(strcmp(">>",args[2])==0) {
        FILE *f=fopen(args[3],"a");
        if(!f) {
            fprintf(stderr,"\033[31mERREUR : impossible d'ouvrir %s\033[0m\n",args[3]);
            return EXIT_SHELL_ERROR;
        }
        fprintf(f,"\n%s",args[1]);
        fclose(f);
        return(EXIT_SHELL_SUCCESS);
    }
    return(EXIT_SHELL_ERROR);
}

int shell_exit(char **args) {
    return EXIT_SHELL_FAILURE; // Indique qu'on quitte
}

int shell_help(char **args) {
    if(!args[1]) {  
        printf("Stash (%s) - Commandes intégrées :\n", OS);
        printf("  cd       : Change de répertoire\n");
        printf("  cat      : Lit un fichier\n");
        printf("  echo     : Ecrit un message\n");
        printf("  exit     : Quitte le shell\n");
        printf("  help     : Affiche cette aide\n");
        printf("  ls       : Affiche la liste des fichiers\n");
        printf("  pwd      : Affiche le répertoire courant\n");
    } else if (strcmp(args[1],"cd")==0) {
        help_cd();
    } else if (strcmp(args[1],"cat")==0) {
        help_cat();
    } else if (strcmp(args[1],"echo")==0) {
        help_echo();
    } else if (strcmp(args[1],"exit")==0) {
        help_exit();
    } else if (strcmp(args[1],"help")==0) {
        help_help();
    } else if (strcmp(args[1],"ls")==0) {
        help_ls();
    } else if (strcmp(args[1],"pwd")==0) {
        help_pwd();
    } else {
        fprintf(stderr,"\033[31mERREUR : '%s' -> Pas une commande valide ou aide non existante.\033[0m\n",args[1]);
        return(EXIT_SHELL_ERROR);
    }
    return EXIT_SHELL_SUCCESS;
}

int shell_ls(char **args) {
    int showHidden = 0;
    int i = 1;
    char path[255];
    strcpy(path, ".");

    while (args[i]) {
        int j = 0;
        if (args[i][j] == '-') {
            while (args[i][j] != '\0') {
                if (args[i][j] == 'h') {
                    showHidden = 1;
                }
                j++;
            }
        } else if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0 || strcmp(args[i], "|") == 0) {
            break;
        } else {
            strcpy(path, args[i]);
        }
        i++;
    }

#ifdef _WIN32
    // ===== Windows version =====
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH];

    snprintf(searchPath, MAX_PATH, "%s\\*", path);
    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "\033[31mERREUR : impossible d'ouvrir le répertoire %s\033[0m\n", searchPath);
        return EXIT_SHELL_ERROR;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            int isHidden = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            if (!showHidden && isHidden) continue;

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                printf("\033[36m'%s'\033[0m\t", findFileData.cFileName);
            } else {
                printf("'%s'\t", findFileData.cFileName);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    printf("\n");
    return EXIT_SHELL_SUCCESS;

#else
    // ===== UNIX version =====
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "\033[31mERREUR : impossible d'ouvrir le répertoire %s\033[0m\n", path);
        return EXIT_SHELL_ERROR;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!showHidden && entry->d_name[0] == '.') {
            continue;
        }
        printf("'%s'\t", entry->d_name);
    }
    closedir(dir);
    printf("\n");
    return EXIT_SHELL_SUCCESS;
#endif
}

int shell_pwd(char **args) {
    printf("%s\n", cwd);
    return EXIT_SHELL_SUCCESS;
}

// ==== TABLEAU DES BUILTINS ==== //
char *builtin_str[] = {
    "cd",
    "cat",
    "echo",
    "exit",
    "help",
    "ls",
    "pwd"
};

int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_execute_builtin(char **args) {
    if (args[0] == NULL) {
        return 1;
    }
    
    if (strcmp(args[0], "cd") == 0) {
        return shell_cd(args);
    } else if (strcmp(args[0], "cat") == 0) {
        return shell_cat(args);
    } else if (strcmp(args[0], "echo") == 0) {
        return shell_echo(args);
    } else if (strcmp(args[0], "exit") == 0) {
        return EXIT_SHELL_FAILURE; // Stop loop
    } else if (strcmp(args[0], "help") == 0) {
        return shell_help(args);
    } else if (strcmp(args[0], "ls") == 0) {
        return shell_ls(args);
    }  else if (strcmp(args[0], "pwd") == 0) {
        return shell_pwd(args);
    }

    return -1; // Not a builtin
}

// ==== EXECUTION COMMANDE EXTERNE ==== //
int shell_launch(char **args) {
#ifdef _WIN32
    char commandLine[SHELL_BUFFSIZE] = "";
    for (int i = 0; args[i] != NULL; i++) {
        strcat(commandLine, args[i]);
        strcat(commandLine, " ");
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd[SHELL_BUFFSIZE];
    strcpy(cmd, commandLine);

    if (!CreateProcess(
        NULL, cmd, NULL, NULL, FALSE, 0,
        NULL, NULL, &si, &pi
    )) {
        fprintf(stderr, "\033[31mErreur : impossible d'exécuter la commande.\033[0m\n");
        return EXIT_SHELL_ERROR;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return EXIT_SHELL_SUCCESS;
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Processus fils
        if (execvp(args[0], args) == -1) {
            perror("Erreur");
        }
        exit(EXIT_SHELL_ERROR);
    } else if (pid < 0) {
        perror("fork");
        return EXIT_SHELL_ERROR;
    } else {
        // Processus parent
        int status;
        waitpid(pid, &status, 0);
        return EXIT_SHELL_SUCCESS;
    }
#endif
}



// ==== SPLIT DE LA LIGNE ==== //
char **shell_split_line(char *line) {
    int bufsize = SHELL_BUFFSIZE;
    char **tokens = malloc(bufsize * sizeof(char *));
    int position = 0;
    int i = 0;

    if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (line[i] != '\0') {
        while (line[i] == ' ') i++;  // Skip spaces

        if (line[i] == '\0') break;

        char *start;
        int in_quotes = 0;

        if (line[i] == '"') {
            in_quotes = 1;
            i++;
            start = &line[i];
            while (line[i] != '\0' && line[i] != '"') i++;
        } else {
            start = &line[i];
            while (line[i] != '\0' && line[i] != ' ') i++;
        }

        int length = &line[i] - start;

        char *token = malloc(length + 1);
        if (!token) {
            fprintf(stderr, "Allocation error\n");
            exit(EXIT_FAILURE);
        }

        strncpy(token, start, length);
        token[length] = '\0';
        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += SHELL_BUFFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "Reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        if (in_quotes && line[i] == '"') i++; // skip closing quote
    }

    tokens[position] = NULL;
    return tokens;
}


// ==== LECTURE LIGNE ==== //
char *shell_read_line(void) {
    static char buffer[SHELL_BUFFSIZE];

    if (fgets(buffer, SHELL_BUFFSIZE, stdin) == NULL) {
        if (feof(stdin)) {
            exit(0); // Ctrl+D
        } else {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    return strdup(buffer); // Alloue dynamiquement pour correspondre au free()
}


// ==== BOUCLE PRINCIPALE ==== //
void shell_loop(void) {
    char *line;
    char **args;
    int status;

    do {
#ifdef _WIN32
        GetCurrentDirectory(MAX_PATH, cwd);
#else
        getcwd(cwd, sizeof(cwd));
#endif
        printf("\033[0;35m%s\033[0m$ ", cwd);
        line = shell_read_line();
        args = shell_split_line(line);

        int ret = shell_execute_builtin(args);

        if (ret == -1) {
            ret = shell_launch(args);
        }

        if (ret == EXIT_SHELL_FAILURE) {
            status = 0;
        } else {
            status = 1;
        }

        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);

    } while (status);
}


void load_env() {
    if (strcmp(OS, "UNIX") == 0) {
        ENV_HOME = getenv("HOME");
    } else {
        ENV_HOME = getenv("USERPROFILE");
    }
}

// ==== MAIN ==== //
int main() {
    
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    load_env();
    printf("\033[1;32mBienvenue dans Stash ! Tapez 'help' pour commencer.\033[0m\n");
    shell_loop();
    return 0;
}

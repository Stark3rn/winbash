#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define OS "WINDOWS"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif
#ifdef __unix__
#define OS "UNIX"
#include <unistd.h>
#endif

#define SHELL_BUFFSIZE 1024
#define EXIT_SHELL_SUCCESS  0
#define EXIT_SHELL_ERROR    1
#define EXIT_SHELL_FAILURE  2


char cwd[MAX_PATH];


// ==== BUILTIN COMMANDES ==== //


int shell_cd(char **args) {
    if(!args[1]) {
        fprintf(stderr, "\033[31mERREUR : Aucun chemin spécifié.\033[0m\n");
        return(EXIT_SHELL_ERROR);
    }
    if (!SetCurrentDirectory(args[1])) {
        fprintf(stderr, "\033[31mERREUR : %s n'est pas un chemin valide.\033[0m\n", args[1]);
        return EXIT_SHELL_ERROR;
    }
    return(EXIT_SHELL_SUCCESS);
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

int shell_exit(char **args) {
    return EXIT_SHELL_FAILURE; // Indique qu'on quitte
}

int shell_help(char **args) {
    printf("Stash (%s) - Commandes intégrées :\n", OS);
    printf("  cd       : Change de répertoire\n");
    printf("  cat      : Lit un fichier\n");
    printf("  exit     : Quitte le shell\n");
    printf("  help     : Affiche cette aide\n");
    printf("  ls       : Affiche la liste des fichiers\n");
    printf("  pwd      : Affiche le répertoire courant\n");
    return EXIT_SHELL_SUCCESS;
}

int shell_ls(char **args) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    char searchPath[MAX_PATH];
    if(!args[1]) {
        snprintf(searchPath, MAX_PATH, ".\\*");
    } else {
        snprintf(searchPath, MAX_PATH, "%s\\*", args[1]);
    }
    
    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr,"\033[31mERREUR : impossible d'ouvrir le répertoire %s\033[0m\n", searchPath);
        return(EXIT_SHELL_ERROR);
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                printf("\033[36m'%s'\033[0m\t", findFileData.cFileName);
            } else {
                printf("'%s'\t", findFileData.cFileName);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    printf("\n");
    return(EXIT_SHELL_SUCCESS);
}

int shell_pwd(char **args) {
    printf("%s\n", cwd);
    return EXIT_SHELL_SUCCESS;
}

// ==== TABLEAU DES BUILTINS ==== //
char *builtin_str[] = {
    "cd",
    "cat",
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
        GetCurrentDirectory(MAX_PATH, cwd);
        printf("\033[0;35m%s\033[0m$ ", cwd);
        line = shell_read_line();
        args = shell_split_line(line);

        int ret = shell_execute_builtin(args);

        if (ret == -1) {
            // Pas une commande interne → tentative externe
            ret = shell_launch(args);
        }

        if (ret == EXIT_SHELL_FAILURE) {
            status = 0; // Quitter la boucle
        } else {
            status = 1; // Continuer
        }

        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);

    } while (status);
}


// ==== MAIN ==== //
int main() {
    
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    printf("\033[1;32mBienvenue dans Stash ! Tapez 'help' pour commencer.\033[0m\n");
    shell_loop();
    return 0;
}

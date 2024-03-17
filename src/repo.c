#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_URL_LENGTH 100

typedef struct {
    char name[MAX_URL_LENGTH];
    char url[MAX_URL_LENGTH];
} Repos;

Repos* read_sources_list(const char* filename, int* num_repos) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    int num_lines = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n')
            num_lines++;
    }
    fseek(file, 0, SEEK_SET);

    Repos* repositories = (Repos*)malloc(num_lines * sizeof(Repos));
    if (repositories == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    *num_repos = 0;
    char line[MAX_URL_LENGTH * 2];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%99s = %99s", repositories[*num_repos].name, repositories[*num_repos].url) == 2) {
            (*num_repos)++;
        }
    }

    fclose(file);

    return repositories;
}

void clone_repositories(Repos* repositories, int num_repos, const char* clone_directory) {
    char clone_command[MAX_URL_LENGTH + 150]; // Increased to accommodate directory path

    for (int i = 0; i < num_repos; i++) {
        char destination[MAX_URL_LENGTH + 50];
        snprintf(destination, sizeof(destination), "%s/%s", clone_directory, repositories[i].name);

        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            sprintf(clone_command, "git clone %s %s", repositories[i].url, destination);
            if (system(clone_command) != 0) {
                fprintf(stderr, "Error cloning repository '%s'\n", repositories[i].name);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to finish
    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0) {
        if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
            fprintf(stderr, "Error cloning repository (pid: %d)\n", wpid);
        }
    }
}
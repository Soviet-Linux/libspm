#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "limits.h"
// class stuff
#include "libspm.h"
#include "cutils.h"
#include <string.h>

char* exec(const char* cmd) 
{
  // Open the command for reading
  FILE* fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command");
    exit(1);
  }
  
  // Read the output a line at a time, and store it
  char path[PATH_MAX];
  char* result = (char*)calloc(2048, sizeof(char));
  while (fgets(path, sizeof(path), fp) != NULL) {
    size_t result_size = malloc_usable_size(result) / sizeof(char);
    size_t result_len = strlen(result);
    size_t path_len = strlen(path);
    if (result_len + path_len > result_size) {
      result = (char*)realloc(result, result_len + path_len + 1);
    }
    strcat(result, path);
  }
  
  // Close
  pclose(fp);
  return result;
}
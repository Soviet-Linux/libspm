#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "limits.h"
// Include custom headers
#include "libspm.h"
#include "cutils.h"
#include <string.h>

// Function to execute a command and capture its output
char* exec(const char* cmd)
{
  // Open the command for reading and capture its output
  FILE* fp = popen(cmd, "r");

  // Check if the command execution failed
  if (fp == NULL) {
    // Print an error message if execution fails
    printf("Failed to run command");
    exit(1);  // Exit the program with an error code
  }

  // Read the output a line at a time and store it
  char path[PATH_MAX];

  // Initialize a result buffer to store the command's output
  char* result = (char*)calloc(2048, sizeof(char));

  // Read the output from the command and append it to the result buffer
  while (fgets(path, sizeof(path), fp) != NULL) {
    // Calculate the current size of the result buffer
    size_t result_size = malloc_usable_size(result) / sizeof(char);

    // Calculate the current length of the result
    size_t result_len = strlen(result);

    // Calculate the length of the current line of output
    size_t path_len = strlen(path);

    // Check if the result buffer needs to be resized to accommodate the output
    if (result_len + path_len > result_size) {
      // Resize the result buffer to ensure it can hold the entire output
      result = (char*)realloc(result, result_len + path_len + 1);
    }

    // Append the current line of output to the result
    strcat(result, path);
  }

  // Close the command execution
  pclose(fp);

  // Return the captured output
  return result;
}

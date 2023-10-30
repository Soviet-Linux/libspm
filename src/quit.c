#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cutils.h"

// Quit the program with the given status code and display an error message if
// status is not 0
/*
Accepts:
- int status: The exit status code.

Description:
This function exits the program with the specified status code. If the status
code is not 0 (indicating an error), it also displays an error message.

Returns:
- void: This function does not return a value.
*/
void quit(int status) {
  if (status != 0) {
    msg(ERROR, "Exiting with status %d", status);
    exit(status);
  }
}

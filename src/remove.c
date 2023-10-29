#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>
#include <unistd.h>

#include "libspm.h"

// Callback function used by nftw to unlink files and directories
/*
Accepts:
- const char *fpath: The path of the file or directory being processed.
- const struct stat *sb: A pointer to a structure containing information about the file.
- int typeflag: A flag indicating the type of the file (file, directory, etc.).
- struct FTW *ftwbuf: A pointer to a structure containing state information for the traversal.

Returns:
- int: An integer indicating the result of the operation.
  - 0: The operation was successful.
  - Non-zero: An error occurred during the operation.

Description:
This function is used as a callback by the nftw function to unlink (remove) files and directories. It attempts to remove the file or directory specified by 'fpath' and returns 0 if the removal is successful. If an error occurs during the removal, it returns a non-zero value and prints an error message indicating the file or directory that caused the error.
*/
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

// Recursively remove a directory and its contents
/*
Accepts:
- char *path: The path of the directory to be removed.

Returns:
- int: An integer indicating the result of the operation.
  - 0: The directory and its contents were successfully removed.
  - Non-zero: An error occurred during the removal.

Description:
This function recursively removes a directory and its contents. It utilizes the nftw function to traverse the directory and its subdirectories, invoking the 'unlink_cb' callback function to unlink and remove files and directories. It returns 0 if the removal is successful and a non-zero value if an error occurs during the operation.
*/
int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

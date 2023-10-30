#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Include custom headers
#include "cutils.h"
#include "data.h"
#include "globals.h"
#include "libspm.h"

// Function to retrieve a package from a data repository
/*
Accepts:
- struct package* i_pkg: A pointer to a package structure with package details.
- const char* out_path: The local path to save the downloaded package.

Returns:
- char*: A pointer to the package format or NULL if there's an error.
*/
char *get(struct package *i_pkg, const char *out_path) {
  // Check if the package name is specified
  if (i_pkg->name == NULL) {
    msg(ERROR, "Package name not specified!");
    return NULL;
  }

  dbg(3, "Loading package %s from ALL_DB\n", i_pkg->name);

  // Allocate memory for package format and section
  char *pkg_format = calloc(64, sizeof(char));
  char *pkg_section = calloc(64, sizeof(char));

  // Retrieve data from the ALL_DB repository
  retrieve_data_repo(ALL_DB, i_pkg, &pkg_format, &pkg_section);

  dbg(3, "Got %s %s %s %s from DB", pkg_format, pkg_section, i_pkg->version,
      i_pkg->type);

  // Check if data retrieval was successful
  if (!pkg_format || !pkg_section || !i_pkg->version || !i_pkg->type) {
    msg(FATAL, "Failed to retrieve data from ALL_DB");
    return NULL;
  }

  dbg(3, "format is '%s'\n", pkg_format);
  dbg(3, "section is '%s'\n", pkg_section);

  dbg(1, "Downloading %s %s %s", i_pkg->name, i_pkg->version, i_pkg->type);

  // Construct the URL for the package download
  char url[64 + strlen(i_pkg->type) + strlen(i_pkg->name) + strlen(pkg_format)];
  sprintf(url, "%s/%s/%s.%s", pkg_section, i_pkg->type, i_pkg->name,
          pkg_format);

  // Download the package to the specified output path
  if (downloadRepo(url, out_path) != 0) {
    msg(ERROR, "Failed to download %s", url);
    return NULL;
  }

  // Free allocated memory
  free(pkg_section);
  return pkg_format;
}

// Function to synchronize the local repository with a remote repository
void sync() {
  // Download the "all.db" file to the specified path
  downloadRepo("all.db", getenv("ALL_DB_PATH"));
}

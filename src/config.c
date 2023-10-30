#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "libspm.h"
#include "globals.h"
#include "cutils.h"

#include <stdbool.h>

#define DEFAULT_CONFIG_FILE "/etc/cccp.conf" // Default config file path

// A hashmap to store config values with default values
typedef struct {
    const char* key;
    const char* default_value;
} ConfigEntry;

ConfigEntry configEntries[] = {
    { "ROOT", "/" },
    { "SOVIET_DEFAULT_FORMAT", "ecmp" },
    { "MAIN_DIR", "/var/cccp" },
    { "WORK_DIR", "/var/cccp/work" },
    { "INSTALLED_DB", "/var/cccp/installed_db" },
    { "ALL_DB", "/var/cccp/all_db" },
    { "CONFIG_FILE", DEFAULT_CONFIG_FILE },
    { "SOVIET_REPOS", "/var/cccp/repos" },
    { "SOVIET_FORMATS", "ecmp" },
    // Add more key-value pairs with default values as needed
};

// The number of entries in the configEntries array
const size_t numConfigEntries = sizeof(configEntries) / sizeof(configEntries[0]);

int readConfig(const char* configFilePath)
{
    if (configFilePath == NULL) {
        configFilePath = DEFAULT_CONFIG_FILE; // Use the default config file path
    }

    dbg(2, "config: %s", configFilePath);
    dbg(2, "config: %s", configFilePath);
    FILE* file = fopen(configFilePath, "r");
    dbg(2, "file is readed");
    if (file == NULL) {
        // File doesn't exist, create it with default values
        file = fopen(configFilePath, "w");
        if (file == NULL) {
            msg(ERROR, "Failed to create config file");
            return 1;
        }

        // Write default values to the config file
        for (size_t i = 0; i < numConfigEntries; i++) {
            fprintf(file, "%s=%s\n", configEntries[i].key, configEntries[i].default_value);
        }

        fclose(file);

        // Reopen the file for reading
        file = fopen(configFilePath, "r");
        if (file == NULL) {
            msg(ERROR, "Failed to open config file for reading");
            return 1;
        }
    }

    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        line[strlen(line) - 1] = 0;

        char* key = strtok(line, "=");
        char* value = strtok(NULL, "=");
        if (key == NULL || value == NULL) {
            msg(ERROR, "Invalid config file");
            fclose(file);
            return 1;
        }

        dbg(3, "Key: %s Value: %s", key, value);

        // Set environment variables based on the key-value pairs in the config file
        setenv(key, value, 1);
    }

    fclose(file);

    // Set environment variables for missing keys with their default values
    for (size_t i = 0; i < numConfigEntries; i++) {
        if (getenv(configEntries[i].key) == NULL) {
            setenv(configEntries[i].key, configEntries[i].default_value, 1);
        }
    }

    return 0;
}

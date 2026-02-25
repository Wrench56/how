#include <stdlib.h>

#include "errors.h"

char* keydb_get(char* service_name) {
    char keyname[256];
    snprintf(keyname, sizeof(keyname), "%s_API_KEY", service_name);
    char* key = getenv(keyname);
    if (key == NULL) {
        FATAL(KEYDB_RETRIEVE_FAIL_EC, "Environment variable \"%s\" has not been set!\n", keyname);
    }

    return key;
} 

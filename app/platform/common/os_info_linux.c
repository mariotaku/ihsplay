#include <string.h>
#include <stdio.h>
#include "util/os_info.h"

static size_t strlen_trim(const char *s);

int os_info_get(os_info_t *info) {
    memset(info, 0, sizeof(*info));
    FILE *fp;
    char name[64], version[64];

    fp = popen("lsb_release -si", "r");
    if (fp != NULL && fgets(name, sizeof(name), fp) != NULL) {
        info->name = strndup(name, strlen_trim(name));
    } else {
        info->name = "Linux";
    }
    if (fp != NULL) {
        pclose(fp);
    }

    fp = popen("lsb_release -sr", "r");
    if (fp == NULL || fgets(version, sizeof(version), fp) == NULL || version_info_parse(&info->version, version) != 0) {
        info->version.major = -1;
        info->version.minor = -1;
        info->version.patch = -1;
    }
    if (fp != NULL) {
        pclose(fp);
    }
    return 0;
}

static size_t strlen_trim(const char *s) {
    size_t len = strlen(s);
    while (len > 0) {
        char ch = s[len - 1];
        if (ch == ' ' || ch == '\n' || ch == '\r') {
            len -= 1;
            continue;
        }
        break;
    }
    return len;
}
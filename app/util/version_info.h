#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct version_info_t {
    int major;
    int minor;
    int patch;
} version_info_t;

typedef struct version_constraint_t {
    version_info_t version;
    enum {
        VERSION_IGNORE,
        VERSION_EQUALS,
        VERSION_NOT_EQUALS,
        VERSION_GREATER_THAN,
        VERSION_GREATER_EQUALS,
        VERSION_LESSER_THAN,
        VERSION_LESSER_EQUALS,
    } operand;
} version_constraint_t;

int version_info_parse(version_info_t *version, const char *value);

bool version_info_valid(const version_info_t *version);

char *version_info_str(version_info_t *version);

int version_info_compare(const version_info_t *a, const version_info_t *b);

int version_constraint_parse(version_constraint_t *constraint, const char *value);

bool version_constraint_check(const version_constraint_t *constraint, const version_info_t *version);
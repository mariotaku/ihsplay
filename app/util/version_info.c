#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "version_info.h"

int version_info_parse(version_info_t *version, const char *value) {
    int n_segs = 0;
    int segs[3] = {-1, -1, -1};
    _Static_assert(sizeof(segs) == sizeof(*version), "Can't cast int[3] to version_info_t");
    const char *rest = value;
    do {
        char *next;
        int num = strtol(rest, &next, 10);
        if (next == rest) {
            break;
        } else if (*next == '.') {
            ++next;
        }
        segs[n_segs++] = num;
        rest = next;
    } while (n_segs < 3);
    if (n_segs == 0) {
        version->major = -1;
        version->minor = -1;
        version->patch = -1;
        return -1;
    }
    *version = *((version_info_t *) segs);
    return 0;
}

bool version_info_valid(const version_info_t *version) {
    return version->major >= 0;
}

char *version_info_str(version_info_t *version) {
    if (version->major < 0) {
        return NULL;
    }
    char tmp[32];
    if (version->minor < 0) {
        snprintf(tmp, 32, "%d", version->major);
    } else if (version->patch < 0) {
        snprintf(tmp, 32, "%d.%d", version->major, version->minor);
    } else {
        snprintf(tmp, 32, "%d.%d.%d", version->major, version->minor, version->patch);
    }
    return strndup(tmp, 32);
}

int version_info_compare(const version_info_t *a, const version_info_t *b) {
    if (a->major < 0 || b->major < 0) {
        return 0;
    }
    int diff = a->major - b->major;
    if (diff != 0) {
        return diff;
    }
    if (a->minor < 0 || b->minor < 0) return 0;
    diff = a->minor - b->minor;
    if (diff != 0) {
        return diff;
    }
    if (a->patch < 0 || b->patch < 0) return 0;
    diff = a->patch - b->patch;
    if (diff != 0) {
        return diff;
    }
    return 0;
}

int version_constraint_parse(version_constraint_t *constraint, const char *value) {
    const char *cur = value;
    while (1) {
        const char *find = strpbrk(cur, "<>=");
        if (find != NULL) {
            cur = find + 1;
        } else {
            break;
        }
    }
    int op_len = cur - value;
    if (op_len > 2) {
        if (strchr("<>=", *(cur - 3)) == NULL) {
            return -1;
        }
        op_len = 2;
    }
    switch (op_len) {
        case 2: {
            if (strncmp(cur - op_len, ">=", op_len) == 0) {
                constraint->operand = VERSION_GREATER_EQUALS;
                return version_info_parse(&constraint->version, cur);
            } else if (strncmp(cur - op_len, "<=", op_len) == 0) {
                constraint->operand = VERSION_LESSER_EQUALS;
                return version_info_parse(&constraint->version, cur);
            } else if (strncmp(cur - op_len, "<>", op_len) == 0) {
                constraint->operand = VERSION_NOT_EQUALS;
                return version_info_parse(&constraint->version, cur);
            }
            break;
        }
        case 1: {
            switch (*(cur - op_len)) {
                case '=':
                    constraint->operand = VERSION_EQUALS;
                    break;
                case '<':
                    constraint->operand = VERSION_LESSER_THAN;
                    break;
                case '>':
                    constraint->operand = VERSION_GREATER_THAN;
                    break;
                default:
                    return -1;
            }
            return version_info_parse(&constraint->version, cur);
        }
        default: {
            // Only possible value is 0
            constraint->operand = VERSION_EQUALS;
            return version_info_parse(&constraint->version, cur);
        }
    }
    return -1;
}

bool version_constraint_check(const version_constraint_t *constraint, const version_info_t *version) {
    switch (constraint->operand) {
        case VERSION_IGNORE:
            return true;
        case VERSION_EQUALS:
            return version_info_compare(version, &constraint->version) == 0;
        case VERSION_NOT_EQUALS:
            return version_info_compare(version, &constraint->version) != 0;
        case VERSION_GREATER_THAN:
            return version_info_compare(version, &constraint->version) > 0;
        case VERSION_GREATER_EQUALS:
            return version_info_compare(version, &constraint->version) >= 0;
        case VERSION_LESSER_THAN:
            return version_info_compare(version, &constraint->version) < 0;
        case VERSION_LESSER_EQUALS:
            return version_info_compare(version, &constraint->version) <= 0;
    }
}
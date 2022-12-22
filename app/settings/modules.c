#include "modules.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "ini.h"
#include "config.h"
#include "util/array_list.h"
#include "app.h"

typedef struct modules_parse_context {
    module_info_t current;
    const os_info_t *os_info;
    array_list_t *modules;
} modules_parse_context;

static void module_info_clear(module_info_t *info);

static void section_changed(modules_parse_context *context);

static void modules_parse_context_destroy(modules_parse_context *context);

static int modules_ini_handler(void *user, const char *section, const char *name, const char *value);

static int module_weight_compare_fn(const void *a, const void *b);

/* Delimited string list */

static int str_list_parse(str_list_t *list, const char *value, const char *delim);

static int str_list_singleton(str_list_t *list, const char *value);

static void str_list_clear(str_list_t *list);

array_list_t *modules_load(const os_info_t *os_info) {
    array_list_t *list = array_list_create(sizeof(module_info_t), 16);
    FILE *f = fopen(SS4S_MODULES_INI_PATH, "r");
    if (f != NULL) {
        modules_parse_context mpc = {.modules = list, .os_info = os_info};
        ini_parse_file(f, modules_ini_handler, &mpc);
        section_changed(&mpc);
        modules_parse_context_destroy(&mpc);
        array_list_qsort(list, module_weight_compare_fn);
    }
    return list;
}

void modules_destroy(array_list_t *list) {
    for (int i = array_list_size(list) - 1; i >= 0; --i) {
        module_info_clear(array_list_get(list, i));
    }
    array_list_destroy(list);
}

bool module_conflicts(const module_info_t *a, const module_info_t *b) {
    for (int i = 0; i < a->conflicts.count; i++) {
        if (strcmp(a->conflicts.elements[i], b->section) == 0) {
            return true;
        }
    }
    for (int i = 0; i < b->conflicts.count; i++) {
        if (strcmp(b->conflicts.elements[i], a->section) == 0) {
            return true;
        }
    }
    return false;
}

static int modules_ini_handler(void *user, const char *section, const char *name, const char *value) {
    modules_parse_context *mpc = user;
    // Only check for same pointer, ignoring content
    if (((section != NULL) != (mpc->current.section != NULL)) ||
        (section != NULL && strcmp(section, mpc->current.section) != 0)) {
        section_changed(mpc);
        mpc->current.section = strdup(section);
    }
    if (strcmp("name", name) == 0) {
        mpc->current.name = strdup(value);
    } else if (strcmp("weight", name) == 0) {
        mpc->current.weight = strtol(value, NULL, 10);
        if (mpc->current.weight < 0) {
            mpc->current.weight = 0;
        } else if (mpc->current.weight > 100) {
            mpc->current.weight = 100;
        }
    } else if (strcmp("audio", name) == 0) {
        mpc->current.has_audio = strcmp("true", value) == 0;
    } else if (strcmp("video", name) == 0) {
        mpc->current.has_video = strcmp("true", value) == 0;
    } else if (strcmp("os_version", name) == 0) {
        version_constraint_parse(&mpc->current.os_version, value);
    } else if (strcmp("modules", name) == 0) {
        str_list_parse(&mpc->current.modules, value, ";");
    } else if (strcmp("conflicts", name) == 0) {
        str_list_parse(&mpc->current.conflicts, value, ";");
    }
    return 1;
}

static void section_changed(modules_parse_context *context) {
    if (context->current.section == NULL) {
        return;
    }
    if (!version_constraint_check(&context->current.os_version, &context->os_info->version)) {
        module_info_clear(&context->current);
        return;
    }
    if (context->current.modules.elements == NULL) {
        str_list_singleton(&context->current.modules, context->current.section);
    }
    module_info_t *info = array_list_add(context->modules, -1);
    *info = context->current;
    memset(&context->current, 0, sizeof(context->current));
}

static void modules_parse_context_destroy(modules_parse_context *context) {
}

static void module_info_clear(module_info_t *info) {
    if (info->section == NULL) {
        return;
    }
    free(info->section);
    if (info->name != NULL) {
        free(info->name);
    }
    str_list_clear(&info->modules);
    str_list_clear(&info->conflicts);
}

static int module_weight_compare_fn(const void *a, const void *b) {
    const module_info_t *m1 = a, *m2 = b;
    int weight_diff = m2->weight - m1->weight;
    return weight_diff;
}

static int str_list_parse(str_list_t *list, const char *value, const char *delim) {
    if (value[0] == 0) {
        return 0;
    }
    int n_delim = 0;
    const char *temp = value;
    do {
        const char *find = strpbrk(temp, delim);
        if (find != NULL) {
            find++;
            n_delim++;
        }
        temp = find;
    } while (temp != NULL);
    /* This array should contain at most n_delim + 1 strings */
    char **items = calloc(n_delim + 1, sizeof(char *));
    char *value_dup = strdup(value);
    char *rest = value_dup, *cur;
    int n_items = 0;
    while ((cur = strtok_r(rest, delim, &rest))) {
        items[n_items++] = cur;
    }
    if (n_items == 0) {
        free(value_dup);
        return 0;
    }
    list->count = n_items;
    list->elements = items;
    list->raw = value_dup;
    return n_items;
}


static int str_list_singleton(str_list_t *list, const char *value) {
    list->elements = calloc(1, sizeof(char *));
    list->elements[0] = strdup(value);
    list->count = 1;
    list->raw = list->elements[0];
    return 1;
}

static void str_list_clear(str_list_t *list) {
    if (list->elements != NULL) {
        free(list->elements);
    }
    if (list->raw != NULL) {
        free(list->raw);
    }
    memset(list, 0, sizeof(str_list_t));
}

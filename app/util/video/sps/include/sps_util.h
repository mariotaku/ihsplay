#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct sps_dimension_t {
    uint16_t width;
    uint16_t height;
} sps_dimension_t;

bool sps_util_parse_dimension_h264(const unsigned char *data, size_t size, sps_dimension_t *dimension);

bool sps_util_parse_dimension_hevc(const unsigned char *data, size_t size, sps_dimension_t *dimension);
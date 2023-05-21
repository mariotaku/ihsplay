#include "util/client_info.h"

#include <string.h>
#include <stdio.h>
#include <pbnjson.h>

#include "lunasynccall.h"
#include "logging.h"

#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>

bool client_info_load(client_info_t *info) {
    memset(info, 0, sizeof(*info));
    char *payload = NULL;
    const char *uri = "luna://com.webos.service.config/getConfigs";
    if (!HLunaServiceCallSync(uri, "{\"configNames\":[\"tv.model.serialnumber\", \"tv.model.modelname\"]}", true,
                              &payload) || !payload) {
        commons_log_warn("OSInfo", "Failed to call %s", uri);
        return client_info_load_default(info);
    }

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, jschema_all(), NULL, NULL);
    jdomparser_ref parser = jdomparser_create(&schemaInfo, 0);
    jdomparser_feed(parser, payload, (int) strlen(payload));
    jdomparser_end(parser);
    jvalue_ref body = jdomparser_get_result(parser);
    jvalue_ref configs = jobject_get(body, j_cstr_to_buffer("configs"));
    if (jis_null(configs)) {
        jdomparser_release(&parser);
        return client_info_load_default(info);
    }
    jvalue_ref serial_number = jobject_get(configs, j_cstr_to_buffer("tv.model.serialnumber"));
    jvalue_ref module_name = jobject_get(configs, j_cstr_to_buffer("tv.model.modelname"));
    if (!jis_string(serial_number) || !jis_string(module_name)) {
        jdomparser_release(&parser);
        return client_info_load_default(info);
    }
    raw_buffer serial_number_buf = jstring_get(serial_number);
    raw_buffer module_name_buf = jstring_get(module_name);

    unsigned char sha1[20];
    mbedtls_sha1((const unsigned char *) serial_number_buf.m_str, serial_number_buf.m_len, sha1);
    memcpy(&info->device_id, &sha1[4], 16);

    mbedtls_sha256((const unsigned char *) serial_number_buf.m_str, serial_number_buf.m_len, info->secret_key, 0);

    char name[64];
    snprintf(name, 64, "webOS TV %.*s", module_name_buf.m_len, module_name_buf.m_str);
    info->name = strndup(name, 64);

    jdomparser_release(&parser);

    info->config.deviceId = info->device_id;
    info->config.secretKey = info->secret_key;
    info->config.deviceName = info->name;

    return true;
}
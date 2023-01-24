#include "util/os_info.h"

#include <string.h>
#include <pbnjson.h>

#include "platform/webos/lunasynccall.h"
#include "logging/app_logging.h"

int os_info_get(os_info_t *info) {
    memset(info, 0, sizeof(*info));
    info->name = strdup("webOS");
    char *payload = NULL;
    const char *uri = "luna://com.webos.service.tv.systemproperty/getSystemInfo";
    if (!HLunaServiceCallSync(uri, "{\"keys\":[\"firmwareVersion\", \"sdkVersion\"]}", true, &payload) || !payload) {
        memset(&info->version, 0, sizeof(version_info_t));
        app_log_warn("OSInfo", "Failed to call %s", uri);
        return -1;
    }

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, jschema_all(), NULL, NULL);
    jdomparser_ref parser = jdomparser_create(&schemaInfo, 0);
    jdomparser_feed(parser, payload, (int) strlen(payload));
    jdomparser_end(parser);
    jvalue_ref os_info = jdomparser_get_result(parser);
    jvalue_ref sdk_version = jobject_get(os_info, j_cstr_to_buffer("sdkVersion"));
    if (jis_null(sdk_version)) {
        memset(&info->version, 0, sizeof(version_info_t));
    } else {
        raw_buffer sdk_version_buf = jstring_get(sdk_version);
        char *version_str = strndup(sdk_version_buf.m_str, sdk_version_buf.m_len);
        version_info_parse(&info->version, version_str);
    }
    jdomparser_release(&parser);
    return 0;
}

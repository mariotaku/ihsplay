#include "error_messages.h"

const char *authorization_result_str(IHS_AuthorizationResult result) {
    switch (result) {
        case IHS_AuthorizationDenied:
            return "Host denied authorization";
        case IHS_AuthorizationNotLoggedIn:
            return "Not logged in";
        case IHS_AuthorizationOffline:
            return "Host is offline";
        case IHS_AuthorizationBusy:
            return "Host is busy";
        case IHS_AuthorizationTimedOut:
            return "Authorization timed out";
        case IHS_AuthorizationCanceled:
            return "Authorization cancelled";
        default:
            return "Unknown error";
    }
}

const char *streaming_result_str(IHS_StreamingResult result) {
    switch (result) {
        case IHS_StreamingUnauthorized: {
            return "Unauthorized";
        }
        case IHS_StreamingScreenLocked: {
            return "Screen is locked";
        }
        case IHS_StreamingBusy: {
            return "Host is busy";
        }
        case IHS_StreamingPINRequired: {
            return "PIN is not supported";
        }
        case IHS_StreamingTimeout: {
            return "Request timed out";
        }
        default: {
            return "Unknown error";
        }
    }
}
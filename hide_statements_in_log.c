#include "postgres.h"
#include "fmgr.h"
#include "utils/guc.h"

PG_MODULE_MAGIC;

void _PG_init(void);

void _PG_fini(void);

static void do_emit_log(ErrorData *errorData);

static void hide_statements_in_log(ErrorData *errorData);

static bool starts_with(const char *string, const char *prefix);

static emit_log_hook_type prev_emit_log_hook = NULL;
static bool guc_delete_log_entry = true;
static char *log_dummy_message = NULL;

void _PG_init(void) {
    DefineCustomBoolVariable("hide_statements_in_log.delete_log",
                             "Delete log entry with statement.",
                             NULL,
                             &guc_delete_log_entry,
                             true,
                             PGC_USERSET,
                             0,
                             NULL,
                             NULL,
                             NULL);
    DefineCustomStringVariable("hide_statements_in_log.dummy_message",
                               "Replace statement in log entry with text.",
                               NULL,
                               &log_dummy_message,
                               "statement is hidden",
                               PGC_USERSET,
                               0,
                               NULL,
                               NULL,
                               NULL);

    prev_emit_log_hook = emit_log_hook;
    emit_log_hook = do_emit_log;
}

void _PG_fini(void) {
    if (emit_log_hook == do_emit_log)
        emit_log_hook = prev_emit_log_hook;
}

static void do_emit_log(ErrorData *errorData) {
    static bool in_hook = false;
    if (prev_emit_log_hook)
        prev_emit_log_hook(errorData);
    if (!in_hook) {
        in_hook = true;
        hide_statements_in_log(errorData);
        in_hook = false;
    }
}

const char *log_starts_with_statement = "statement: ";
const char *log_starts_with_duration = "duration: ";
const char *detail_log_starts_with_failed = "Failed process was running: ";

static void hide_statements_in_log(ErrorData *errorData) {
    errorData->hide_stmt = true;
    errorData->hide_ctx = true;

    bool found_in_message = false;
    if (errorData->elevel == LOG) {
        if (errorData->message) {
            if (starts_with(errorData->message, log_starts_with_statement)) {
                found_in_message = true;
            } else if (starts_with(errorData->message, log_starts_with_duration)) {
                found_in_message = true;
            }
        }
    }
    if (found_in_message) {
        if (guc_delete_log_entry) {
            errorData->output_to_server = false;
        } else {
            pfree(errorData->message);
            errorData->message = pstrdup(log_dummy_message);
        }
    }

    bool found_in_detail = false;
    if (errorData->detail) {
        found_in_detail = starts_with(errorData->detail, detail_log_starts_with_failed);
    }
    if (found_in_detail) {
        if (guc_delete_log_entry) {
            errorData->output_to_server = false;
        } else {
            pfree(errorData->detail);
            errorData->detail = pstrdup(log_dummy_message);
        }
    }
}

static bool starts_with(const char *string, const char *prefix) {
    if (strlen(string) < strlen(prefix)) {
        return false;
    }
    for (size_t i = 0; prefix[i] != '\0'; i++) {
        if (string[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

#include "postgres.h"
#include "fmgr.h"
#include "utils/guc.h"

PG_MODULE_MAGIC;

void _PG_init(void);

void _PG_fini(void);

static void do_emit_log(ErrorData *errorData);

static void hide_statements_in_log(ErrorData *errorData);

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
                               "message is hidden",
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

#define CLEAN_ERROR_DATA(errorData, target_field) \
    { \
        if (guc_delete_log_entry) { \
            errorData->output_to_server = false; \
        } else { \
            pfree(errorData->target_field); \
            errorData->target_field = pstrdup(log_dummy_message); \
        } \
    }

static void hide_statements_in_log(ErrorData *errorData) {
    errorData->hide_stmt = true;
    errorData->hide_ctx = true;

    if (errorData->elevel == LOG) {
        if (errorData->message) {
            CLEAN_ERROR_DATA(errorData, message);
        }
    }
    if (errorData->detail) {
        CLEAN_ERROR_DATA(errorData, detail);
    }
}

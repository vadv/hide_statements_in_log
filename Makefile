MODULES = hide_statements_in_log

EXTENSION = hide_statements_in_log
PGFILEDESC = "hide_statements_in_log - hide statements in postgresql log"

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

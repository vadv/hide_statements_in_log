BEGIN;

LOAD 'hide_statements_in_log';
SET log_min_duration_statement to 0;
SET log_statement to "all";

SELECT plan(1);
SELECT lives_ok(
    'SELECT pg_sleep(1)',
    'Slow query');
SELECT * FROM finish();
ROLLBACK;
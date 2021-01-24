# About

`hide_statements_in_log` is a simple PostgreSQL extensions which hides sensitive
information from logs like query and plans.

## Usage

This extension is designed for use in shared_preload_libraries.

## Settings

```
-> select name, short_desc, boot_val from pg_settings where name like 'hide_statements_in_log%';

-[ RECORD 1 ]-----------------------------------------
name       | hide_statements_in_log.delete_log
short_desc | Delete log entry with statement.
boot_val   | on

-[ RECORD 2 ]-----------------------------------------
name       | hide_statements_in_log.dummy_message
short_desc | Replace statement in log entry with text.
boot_val   | statement is hidden
```

## Build and install

```bash
$ # export PG_CONFIG=/path/to/pg_config
$ make && make install
```

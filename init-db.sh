#!/bin/bash
set -e

clickhouse client -n <<-EOSQL
    CREATE TABLE Log
    (
     timestamp      UInt64,
     message        String,
     log_owner      Tuple(host_name String, app_name String),
     log_entry      Tuple(log_level String, message String),
    )
    ENGINE = MergeTree
    ORDER BY timestamp;
EOSQL

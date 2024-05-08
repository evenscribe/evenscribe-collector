#!/bin/bash
set -e

clickhouse client -n <<-EOSQL
    CREATE TABLE logs
    (
        message String,
    )
    ENGINE = MergeTree
    PRIMARY KEY (message);
EOSQL

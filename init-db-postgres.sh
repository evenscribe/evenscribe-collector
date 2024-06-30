#!/bin/bash
set -e

psql <<-EOSQL
    CREATE DATABASE evenscribe_db;

    \c evenscribe_db;

    DROP TABLE IF EXISTS logs;

    CREATE TABLE logs
    (
        Timestamp INT,
        TraceId VARCHAR(255),
        SpanId VARCHAR(255),
        TraceFlags INT,
        SeverityText VARCHAR(255),
        SeverityNumber INT,
        ServiceName VARCHAR(255),
        Body TEXT,
        ResourceAttributes JSONB,
        LogAttributes JSONB
    );
EOSQL

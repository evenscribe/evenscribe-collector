#!/bin/bash
set -e

clickhouse client -n <<-EOSQL
    CREATE TABLE IF NOT EXISTS logs
    (
        Timestamp DateTime CODEC(DoubleDelta, ZSTD(1)),
        TraceId String,
        SpanId String,
        TraceFlags UInt32,
        SeverityText LowCardinality(String),
        SeverityNumber Int32,
        ServiceName LowCardinality(String),
        Body String,
        ResourceAttributes Map(LowCardinality(String), String),
        LogAttributes Map(LowCardinality(String), String),
        # INDEX idx_trace_id TraceId TYPE bloom_filter GRANULARITY 1,
        # INDEX idx_res_attr_key mapKeys(ResourceAttributes) TYPE bloom_filter GRANULARITY 1,
        # INDEX idx_res_attr_value mapValues(ResourceAttributes) TYPE bloom_filter GRANULARITY 1,
        # INDEX idx_log_attr_key mapKeys(LogAttributes) TYPE bloom_filter GRANULARITY 1,
        # INDEX idx_log_attr_value mapValues(LogAttributes) TYPE bloom_filter GRANULARITY 1,
        # INDEX idx_body Body TYPE tokenbf_v1(32768, 3, 0) GRANULARITY 1
    )
    engine MergeTree()
        partition by toDate(Timestamp)
        order by (ServiceName, SeverityText, toUnixTimestamp(Timestamp), TraceId);
EOSQL

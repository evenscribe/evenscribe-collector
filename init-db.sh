#!/bin/bash
set -e

clickhouse client -n <<-EOSQL
    CREATE TABLE IF NOT EXISTS logs
    (
        Timestamp DateTime64(9),
        TraceId String,
        SpanId String,
        TraceFlags UInt32,
        SeverityText LowCardinality(String),
        SeverityNumber Int32,
        ServiceName LowCardinality(String),
        Body String,
        ResourceAttributes Map(LowCardinality(String), String),
        LogAttributes Map(LowCardinality(String), String),
        INDEX idx_trace_id TraceId TYPE bloom_filter GRANULARITY 1,
        INDEX idx_res_attr_key mapKeys(ResourceAttributes) TYPE bloom_filter GRANULARITY 1,
        INDEX idx_res_attr_value mapValues(ResourceAttributes) TYPE bloom_filter GRANULARITY 1,
        INDEX idx_log_attr_key mapKeys(LogAttributes) TYPE bloom_filter GRANULARITY 1,
        INDEX idx_log_attr_value mapValues(LogAttributes) TYPE bloom_filter GRANULARITY 1,
        INDEX idx_body Body TYPE tokenbf_v1(32768, 3, 0) GRANULARITY 1
    )
    engine MergeTree()
        partition by toDate(Timestamp)
        order by (ServiceName, SeverityText, toUnixTimestamp(Timestamp), TraceId);

    INSERT INTO logs VALUES
        (toDateTime64('2024-05-27 00:01:02.123456789', 9), '0a1b2c3d4e5f6a7b', 'a1b2c3d4e5f6a7b8', 1, 'INFO', 1, 'auth-service', 'User login successful', {'host': 'auth-server-1', 'region': 'us-west'}, {'user_id': '12345', 'status': 'successful'}),
        (toDateTime64('2024-05-27 00:02:03.234567890', 9), '1a2b3c4d5e6f7a8b', 'b1c2d3e4f5g6h7i8', 1, 'ERROR', 3, 'payment-service', 'Payment failed due to insufficient funds', {'host': 'payment-server-1', 'region': 'us-east'}, {'transaction_id': '67890', 'error_code': '101'}),
        (toDateTime64('2024-05-27 00:03:04.345678901', 9), '2a3b4c5d6e7f8a9b', 'c1d2e3f4g5h6i7j8', 1, 'WARN', 2, 'order-service', 'Order shipment delayed', {'host': 'order-server-1', 'region': 'eu-central'}, {'order_id': '54321', 'status': 'delayed'}),
        (toDateTime64('2024-05-27 00:04:05.456789012', 9), '3a4b5c6d7e8f9a0b', 'd1e2f3g4h5i6j7k8', 1, 'INFO', 1, 'auth-service', 'User password reset', {'host': 'auth-server-2', 'region': 'us-west'}, {'user_id': '12346', 'status': 'reset'}),
        (toDateTime64('2024-05-27 00:05:06.567890123', 9), '4a5b6c7d8e9f0a1b', 'e1f2g3h4i5j6k7l8', 1, 'ERROR', 3, 'payment-service', 'Payment declined by bank', {'host': 'payment-server-2', 'region': 'us-east'}, {'transaction_id': '67891', 'error_code': '102'}),
        (toDateTime64('2024-05-27 00:06:07.678901234', 9), '5a6b7c8d9e0f1a2b', 'f1g2h3i4j5k6l7m8', 1, 'WARN', 2, 'order-service', 'Order address verification failed', {'host': 'order-server-2', 'region': 'eu-central'}, {'order_id': '54322', 'status': 'failed'}),
        (toDateTime64('2024-05-27 00:07:08.789012345', 9), '6a7b8c9d0e1f2a3b', 'g1h2i3j4k5l6m7n8', 1, 'INFO', 1, 'auth-service', 'User email updated', {'host': 'auth-server-3', 'region': 'us-west'}, {'user_id': '12347', 'status': 'updated'}),
        (toDateTime64('2024-05-27 00:08:09.890123456', 9), '7a8b9c0d1e2f3a4b', 'h1i2j3k4l5m6n7o8', 1, 'ERROR', 3, 'payment-service', 'Payment processing error', {'host': 'payment-server-3', 'region': 'us-east'}, {'transaction_id': '67892', 'error_code': '103'}),
        (toDateTime64('2024-05-27 00:09:10.901234567', 9), '8a9b0c1d2e3f4a5b', 'i1j2k3l4m5n6o7p8', 1, 'WARN', 2, 'order-service', 'Order payment pending', {'host': 'order-server-3', 'region': 'eu-central'}, {'order_id': '54323', 'status': 'pending'}),
        (toDateTime64('2024-05-27 00:10:11.012345678', 9), '9a0b1c2d3e4f5a6b', 'j1k2l3m4n5o6p7q8', 1, 'INFO', 1, 'auth-service', 'User profile updated', {'host': 'auth-server-4', 'region': 'us-west'}, {'user_id': '12348', 'status': 'updated'}),
        (toDateTime64('2024-05-27 00:11:12.123456789', 9), '0b1c2d3e4f5a6b7c', 'k1l2m3n4o5p6q7r8', 1, 'ERROR', 3, 'payment-service', 'Transaction timed out', {'host': 'payment-server-4', 'region': 'us-east'}, {'transaction_id': '67893', 'error_code': '104'}),
        (toDateTime64('2024-05-27 00:12:13.234567890', 9), '1b2c3d4e5f6a7b8c', 'l1m2n3o4p5q6r7s8', 1, 'WARN', 2, 'order-service', 'Order return initiated', {'host': 'order-server-4', 'region': 'eu-central'}, {'order_id': '54324', 'status': 'initiated'}),
        (toDateTime64('2024-05-27 00:13:14.345678901', 9), '2b3c4d5e6f7a8b9c', 'm1n2o3p4q5r6s7t8', 1, 'INFO', 1, 'auth-service', 'User two-factor authentication enabled', {'host': 'auth-server-5', 'region': 'us-west'}, {'user_id': '12349', 'status': 'enabled'}),
        (toDateTime64('2024-05-27 00:14:15.456789012', 9), '3b4c5d6e7f8a9b0c', 'n1o2p3q4r5s6t7u8', 1, 'ERROR', 3, 'payment-service', 'Card expired', {'host': 'payment-server-5', 'region': 'us-east'}, {'transaction_id': '67894', 'error_code': '105'}),
        (toDateTime64('2024-05-27 00:15:16.567890123', 9), '4b5c6d7e8f9a0b1c', 'o1p2q3r4s5t6u7v8', 1, 'WARN', 2, 'order-service', 'Order cancellation requested', {'host': 'order-server-5', 'region': 'eu-central'}, {'order_id': '54325', 'status': 'requested'}),
        (toDateTime64('2024-05-27 00:16:17.678901234', 9), '5b6c7d8e9f0a1b2c', 'p1q2r3s4t5u6v7w8', 1, 'INFO', 1, 'auth-service', 'User profile picture updated', {'host': 'auth-server-6', 'region': 'us-west'}, {'user_id': '12350', 'status': 'updated'}),
        (toDateTime64('2024-05-27 00:17:18.789012345', 9), '6b7c8d9e0f1a2b3c', 'q1r2s3t4u5v6w7x8', 1, 'ERROR', 3, 'payment-service', 'Payment gateway timeout', {'host': 'payment-server-6', 'region': 'us-east'}, {'transaction_id': '67895', 'error_code': '106'}),
        (toDateTime64('2024-05-27 00:18:19.890123456', 9), '7b8c9d0e1f2a3b4c', 'r1s2t3u4v5w6x7y8', 1, 'WARN', 2, 'order-service', 'Order address updated', {'host': 'order-server-6', 'region': 'eu-central'}, {'order_id': '54326', 'status': 'updated'}),
        (toDateTime64('2024-05-27 00:19:20.901234567', 9), '8b9c0d1e2f3a4b5c', 's1t2u3v4w5x6y7z8', 1, 'INFO', 1, 'auth-service', 'User email verified', {'host': 'auth-server-7', 'region': 'us-west'}, {'user_id': '12351', 'status': 'verified'}),
        (toDateTime64('2024-05-27 00:20:21.012345678', 9), '9b0c1d2e3f4a5b6c', 't1u2v3w4x5y6z7a8', 1, 'ERROR', 3, 'payment-service', 'Payment authorization failed', {'host': 'payment-server-7', 'region': 'us-east'}, {'transaction_id': '67896', 'error_code': '107'}),
        (toDateTime64('2024-05-27 00:21:22.123456789', 9), '0c1d2e3f4a5b6c7d', 'u1v2w3x4y5z6a7b8', 1, 'WARN', 2, 'order-service', 'Order payment confirmed', {'host': 'order-server-7', 'region': 'eu-central'}, {'order_id': '54327', 'status': 'confirmed'}),
        (toDateTime64('2024-05-27 00:22:23.234567890', 9), '1c2d3e4f5a6b7c8d', 'v1w2x3y4z5a6b7c8', 1, 'INFO', 1, 'auth-service', 'User password changed', {'host': 'auth-server-8', 'region': 'us-west'}, {'user_id': '12352', 'status': 'changed'});
EOSQL

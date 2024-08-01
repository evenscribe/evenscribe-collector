# Evenscribe

## How to Install

`brew install evenscribe/homebrew-evenscribe/evenscribe-collector@0.1`

## Add the following to your ~/.evenscriberc file:

```json
{
  "host": "YOUR_DB_HOST",
  "port": "YOUR_DB_PORT",
  "user": "YOUR_DB_USERNAME",
  "password": "YOUR_DB_PASSWORD",
  "dbname": "YOUR_DB_NAME",
  "database_kind": "clickhouse || postgres"
}
```

## Clients

|Language|Link|
|---|--|
|Go|[evenscribe-js](https://github.com/evenscribe/evenscribe-go)|
|Javascript/Typescript|[evenscribe-go](https://github.com/evenscribe/evenscribe-js)|
|Rust|[evenscribe-rs](https://github.com/evenscribe/Olympus.rs)|

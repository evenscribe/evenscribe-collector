FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake \
    clang \
    build-essential \
    libpq-dev \
    libssl-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY ./.evenscriberc /root/.evenscriberc

WORKDIR /app
COPY . .

RUN cmake -DWITH_OPENSSL=ON -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql/ -B build -S .
RUN cmake --build build -j$(nproc)

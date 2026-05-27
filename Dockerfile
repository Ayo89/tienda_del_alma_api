FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    uuid-dev \
    libssl-dev \
    libcpprest-dev \
    libsodium-dev \
    libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --config Release

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    libcpprest2.10 \
    libsodium23 \
    libmysqlclient21 \
    uuid-runtime \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/tienda_del_alma /app/tienda_del_alma

EXPOSE 8080

CMD ["./tienda_del_alma"]
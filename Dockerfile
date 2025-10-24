FROM --platform=linux/amd64 debian:stable-slim AS builder
RUN apt-get update && apt-get install -y build-essential g++ cmake make && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . /src
RUN make all

FROM --platform=linux/amd64 debian:stable-slim
RUN apt-get update && apt-get install -y ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /src/bin /app/bin
RUN chmod +x /app/bin/* || true
ENV PATH="/app/bin:${PATH}"
VOLUME ["/data"]
ENTRYPOINT ["/bin/bash"]FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
      g++ make cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# compile usando Makefile padrão (ajuste conforme seu build)
RUN make build

# diretório para dados persistentes (montado como volume)
VOLUME ["/data"]

# variáveis de ambiente para facilitar testes
ENV CSV_PATH=/data/input.csv \
    DATA_DIR=/data/db \
    LOG_LEVEL=info

# por padrão, mostra ajuda dos binários
CMD ["bash", "-lc", "echo 'Use: docker run ... upload|findrec|seek1|seek2'; ls -l bin/"]

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
      g++ make cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# compile usando Makefile padrão (ajustado para chamar 'make build')
# ensure any host-built bin/ copied by the `COPY` step is removed so
# the container builds artifacts with the container toolchain
RUN rm -rf bin *.o src/*.o || true && make build

# diretório para dados persistentes (montado como volume)
VOLUME ["/data"]

# variáveis de ambiente para facilitar testes
ENV CSV_PATH=/data/input.csv \
    DATA_DIR=/data/db \
    LOG_LEVEL=info

# por padrão, mostra ajuda dos binários
CMD ["bash", "-lc", "echo 'Use: docker run --rm -v $(pwd)/data:/data tp2 ./bin/upload /data/input.csv'; ls -l bin/"]

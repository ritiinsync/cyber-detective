FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends g++ ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY server.cpp index.html ./
RUN g++ -std=c++17 -O2 server.cpp -o cyber_server
ENV PORT=10000
EXPOSE 10000
CMD ["./cyber_server"]

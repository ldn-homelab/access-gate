# build access-gate binary
FROM docker.io/library/alpine:3.20 AS builder
RUN apk add --no-cache gcc musl-dev sqlite-dev
WORKDIR /build
COPY access-gate.c .
COPY include/ include/
COPY src/ src/
RUN gcc -O2 -Iinclude -o access-gate access-gate.c src/*.c -lsqlite3

# create image and execute access-gate binary
FROM docker.io/library/alpine:3.20
RUN apk add --no-cache sqlite-libs
COPY --from=builder /build/access-gate /access-gate
ENTRYPOINT ["/access-gate"]
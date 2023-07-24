FROM debian:12-slim AS builder

ENV DEBIAN_FRONTEND=noninteractive \
    DEBCONF_NONINTERACTIVE_SEEN=true \
    LC_ALL=C.UTF-8 \
    LANG=C.UTF-8

RUN apt-get update \
 && apt-get install -y \
      build-essential \
      autoconf \
      libtool \
      git \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app
RUN autoreconf -i \
 && ./configure \
      --disable-docs \
      --disable-valgrind \
      --with-oniguruma=builtin \
      --enable-static \
      --enable-all-static \
      --prefix=/usr/local \
 && make -j$(nproc) \
 && make check VERBOSE=yes \
 && make install-strip

FROM scratch

COPY --from=builder /app/AUTHORS /app/COPYING /usr/local/bin/jq /
RUN ["/jq", "--version"]
ENTRYPOINT ["/jq"]

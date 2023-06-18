FROM alpine:3.18 AS builder

COPY . /app

# get dependencies, build, and remove anything we don't need for running jq.

RUN apk update && \
    apk add --no-cache --virtual .build-deps \
        build-base \
        autoconf \
        automake \
        libtool \
        git \
        bison \
        flex && \
    (cd /app && \
        git submodule update --init && \
        autoreconf -if && \
        ./configure --disable-docs --enable-all-static CFLAGS='-Os -static -no-pie' --prefix=/usr/local && \
        make -j8 && \
        make check && \
        make install-strip ) && \
    (cd /app/modules/oniguruma && \
        make uninstall ) && \
    (cd /app && \
        make distclean ) && \
    apk del .build-deps && \
    rm -rf /app/modules/oniguruma/{*,.git,.gitignore} && \
    rm -rf /var/lib/gems

FROM scratch

COPY --from=builder /app/AUTHORS /app/COPYING /usr/local/bin/jq /

ENTRYPOINT ["/jq"]
CMD []

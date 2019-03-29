FROM debian:9

ENV DEBIAN_FRONTEND=noninteractive \
    DEBCONF_NONINTERACTIVE_SEEN=true \
    LC_ALL=C.UTF-8 \
    LANG=C.UTF-8

COPY . /app

# get dependencies, build, and remove anything we don't need for running jq.
# valgrind seems to have trouble with pthreads TLS so it's off.

RUN apt-get update && \
    apt-get install -y \
        build-essential \
        autoconf \
        libtool \
        git \
        bison \
        flex \
        python3 \
        python3-pip \
        wget && \
    pip3 install pipenv && \
    (cd /app/docs && pipenv sync) && \
    (cd /app && \
        git submodule init && \
        git submodule update && \
        autoreconf -i && \
        ./configure --disable-valgrind --enable-all-static --prefix=/usr/local && \
        make -j8 && \
        make check && \
        make install ) && \
    (cd /app/modules/oniguruma && \
        make uninstall ) && \
    (cd /app && \
        make distclean ) && \
    apt-get purge -y \
        build-essential \
        autoconf \
        libtool \
        bison \
        git \
        flex \
        python3 \
        python3-pip && \
    apt-get autoremove -y && \
    rm -rf /app/modules/oniguruma/* && \
    rm -rf /app/modules/oniguruma/.git && \
    rm -rf /app/modules/oniguruma/.gitignore && \
    rm -rf /var/lib/apt/lists/* /var/lib/gems

ENTRYPOINT ["/usr/local/bin/jq"]
CMD []

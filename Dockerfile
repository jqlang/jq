FROM debian:8

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
        ruby \
        wget \
        ruby-dev && \
    git submodule init && \
    git submodule update && \
    gem install bundler && \
    (cd /app/docs && bundle install) && \
    (cd /app && \
        autoreconf -i && \
        ./configure --with-oniguruma=builtin --disable-valgrind --enable-all-static --prefix=/usr/local && \
        make -j8 && \
        make check && \
        make install && \
        make distclean ) && \
    apt-get purge -y \
        build-essential \
        autoconf \
        libtool \
        bison \
        git \
        flex \
        ruby \
        ruby-dev && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/* /var/lib/gems

ENTRYPOINT ["/usr/local/bin/jq"]
CMD []

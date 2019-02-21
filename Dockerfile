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
        python3 \
        wget && \
    wget https://github.com/kkos/oniguruma/archive/v5.9.6.tar.gz -O onig-5.9.6.tar.gz && \
    sha512sum onig-5.9.6.tar.gz | grep 6b048d345e148c9da38af7a8df76d4a358eb3d4db91fa7834076e511f526a63544284f212b0d56badbbd33112c8b458a5fff02bfbbda012ecfe478bc436ea679 && \
    tar zxvf onig-5.9.6.tar.gz && \
    (cd oniguruma-5.9.6 && \
        touch NEWS ChangeLog && \
        autoreconf -i && \
        ./configure --prefix=/usr/local && \
        make && \
        make install ) && \
    pip3 install pipenv && \
    (cd /app/docs && pipenv sync) && \
    (cd /app && \
        autoreconf -i && \
        ./configure --disable-valgrind --enable-all-static --prefix=/usr/local && \
        make -j8 && \
        make check && \
        make install && \
        make distclean ) && \
    (cd oniguruma-5.9.6 && \
        make uninstall ) && \
    apt-get purge -y \
        build-essential \
        autoconf \
        libtool \
        bison \
        git \
        flex \
        python3 && \
    apt-get autoremove -y && \
    rm -rf oniguruma-5.9.6 && \
    rm -rf /var/lib/apt/lists/* /var/lib/gems

ENTRYPOINT ["/usr/local/bin/jq"]
CMD []

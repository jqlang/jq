FROM debian:12 AS builder

ENV DEBIAN_FRONTEND=noninteractive \
    DEBCONF_NONINTERACTIVE_SEEN=true \
    LC_ALL=C.UTF-8 \
    LANG=C.UTF-8

COPY . /app

# get dependencies, build, and remove anything we don't need for running jq.
# source command doesn't exist in dash 
SHELL ["/bin/bash", "-c"]

RUN apt update && \
    apt install -y \
        build-essential \
        autoconf \
        libtool \
        git \
        bison \
        flex \
        python3.11-minimal \
        python3-pip \
        python3.11-venv \
        wget && \
    (cd /app/docs && \
        python3 -m venv create /tmp/env && source $_/bin/activate && pip install pipenv && pipenv sync ) && \
    (cd /app && \
        git submodule update --init && \
        autoreconf -i && \
        ./configure --enable-all-static --prefix=/usr/local && \
        make -j8 && \
        make check && \
        make install ) && \
    (cd /app/modules/oniguruma && \
        make uninstall ) && \
    (cd /app && \
        make distclean ) && \
    apt purge -y \
        build-essential \
        autoconf \
        libtool \
        bison \
        git \
        flex \
        python3.11-minimal \
        python3-pip \
        python3.11-venv && \
    apt autoremove -y && \
    rm -rf /app/modules/oniguruma/{*,.git,.gitignore} && \
    rm -rf /var/lib/apt/lists/* /var/lib/gems

FROM debian:12

COPY --from=builder /usr/local/bin/jq /usr/local/bin/

ENTRYPOINT ["/usr/local/bin/jq"]
CMD []

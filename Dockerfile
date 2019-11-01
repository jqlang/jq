
ARG PYTHON_VERSION=3.6.7

FROM python:${PYTHON_VERSION}-stretch AS build
CMD ["bash"]
ENV LC_ALL=C.UTF-8
ARG DEBIAN_FRONTEND=noninteractive
ARG DEBCONF_NONINTERACTIVE_SEEN=true
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        autoconf \
        bison \
        build-essential \
        flex \
        git \
        libtool \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Install docs dependencies; we're only copying the Pipfile here,
# so that this is only run again if dependencies change
RUN  pip3 install pipenv
COPY docs/Pipfile docs/Pipfile.lock ./docs/
RUN  cd docs && pipenv sync

COPY .gitmodules .
COPY .git        ./.git/
RUN git submodule init
RUN git submodule update
RUN sed -i.bak '/^AM_INIT_AUTOMAKE(\[-Wno-portability 1\.14\])$/s/14/11/' modules/oniguruma/configure.ac

COPY . .
RUN autoreconf -if
RUN ./configure --disable-valgrind --with-oniguruma=builtin YACC=/usr/bin/bison
RUN make BISON_PKGDATADIR=/usr/bin/bison src/parser.c || make src/parser.c
RUN make -j8
RUN make check -j8
RUN make DESTDIR=/build install
RUN /build/usr/local/bin/jq -V

# Collect the files for the final image
FROM debian:stretch-slim AS deploy
ENV LC_ALL=C.UTF-8
ARG DEBIAN_FRONTEND=noninteractive
ARG DEBCONF_NONINTERACTIVE_SEEN=true
RUN apt-get update \
 && apt-get install -y --no-install-recommends man \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build /build/usr/local/.  /usr/local/
RUN jq -V
ENTRYPOINT ["/usr/local/bin/jq"]
CMD ["--help"]

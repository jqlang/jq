FROM alpine:3.12 AS build

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN apk add --no-cache \
        automake \
        autoconf \
        build-base \
        bison \
        flex \
        git \
        libtool \
        oniguruma-dev

WORKDIR /app
COPY . .
RUN autoreconf -if
RUN ./configure --disable-docs --enable-all-static CFLAGS='-Os -static -no-pie' CXXFLAGS='-Os -static -no-pie'
RUN make
RUN make check
RUN strip jq

# Ensure that the built executable is really statically linked.
RUN file jq | grep -Fw 'statically linked'

# The deploy stage is the final image, and only contains artefacts
# that should be published.
FROM scratch AS deploy
COPY --from=build /app/AUTHORS /
COPY --from=build /app/COPYING /
COPY --from=build /app/jq      /jq
RUN ["/jq", "-V"]
ENTRYPOINT ["/jq"]
CMD ["--help"]

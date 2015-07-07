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
    wget http://www.geocities.jp/kosako3/oniguruma/archive/onig-5.9.6.tar.gz && \
    sha512sum onig-5.9.6.tar.gz | grep 4a181ea6f0e1a018bbaf6c87e666dfffd1ef4b8f5dcead07fa0b6564a76174e7e01854173924668433ae74c455dbced6a0e1b43e9066f0499b4a57e855e1a2b2 && \
    tar zxvf onig-5.9.6.tar.gz && \
    (cd onig-5.9.6 && \
        ./configure --prefix=/usr/local && \
        make && \
	make install ) && \
    gem install bundler && \
    (cd /app/docs && bundle install) && \
    (cd /app && \
        autoreconf -i && \
        ./configure --disable-valgrind --enable-all-static --prefix=/usr/local && \
	make -j8 && \
	make check && \
	make install && \
	make distclean ) && \
    (cd onig-5.9.6 && \
        make uninstall ) && \
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
    rm -rf onig-5.9.6 && \
    rm -rf /var/lib/apt/lists/* /var/lib/gems

ENTRYPOINT ["/usr/local/bin/jq"]

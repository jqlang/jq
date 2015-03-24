FROM debian:wheezy

# get dependencies

RUN apt-get update

RUN apt-get install -y build-essential
RUN apt-get install -y autoconf
RUN apt-get install -y libonig2
RUN apt-get install -y libtool
RUN apt-get install -y git
RUN apt-get install -y valgrind
RUN apt-get install -y bison
RUN apt-get install -y flex
RUN apt-get install -y ruby1.9.3

RUN gem install bundler

# get docs dependencies

COPY ./docs/Gemfile /app/docs/Gemfile
COPY ./docs/Gemfile.lock /app/docs/Gemfile.lock

WORKDIR /app/docs

RUN bundle install

# copy files

WORKDIR /app

COPY . /app

# build

RUN autoreconf -i
RUN ./configure
RUN make -j8
RUN make check

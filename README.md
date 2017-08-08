jq
==

jq is a lightweight and flexible command-line JSON processor.

[![Coverage Status](https://coveralls.io/repos/stedolan/jq/badge.svg?branch=master&service=github)](https://coveralls.io/github/stedolan/jq?branch=master),
Unix: [![Build Status](https://travis-ci.org/stedolan/jq.svg?branch=master)](https://travis-ci.org/stedolan/jq),
Windows: [![Windows build status](https://ci.appveyor.com/api/projects/status/mi816811c9e9mx29?svg=true)](https://ci.appveyor.com/project/stedolan/jq)


If you want to learn to use jq, read the documentation at
[https://stedolan.github.io/jq](https://stedolan.github.io/jq).  This
documentation is generated from the docs/ folder of this repository.
You can also try it online at [jqplay.org](https://jqplay.org).

If you want to hack on jq, feel free, but be warned that its internals
are not well-documented at the moment. Bring a hard hat and a
shovel.  Also, read the wiki: https://github.com/stedolan/jq/wiki, where
you will find cookbooks, discussion of advanced topics, internals,
release engineering, and more.

Source tarball and built executable releases can be found on the
homepage and on the github release page, https://github.com/stedolan/jq/releases

If you're building directly from the latest git, you'll need flex,
bison (3.0 or newer), libtool, make, and autoconf installed.
To get regexp support you'll also need to install Oniguruma or clone it as a
git submodule as per the instructions below.
(note that jq's tests require regexp support to pass).  To build, run:

    git submodule update --init # if building from git to get oniguruma
    autoreconf -fi              # if building from git
    ./configure --with-oniguruma=builtin
    make -j8
    make check

To build without bison or flex, add `--disable-maintainer-mode` to the
./configure invocation:

    ./configure --with-oniguruma=builtin --disable-maintainer-mode

(Developers must not use `--disable-maintainer-mode`, not when making
changes to the jq parser and/or lexer.)

To build a statically linked version of jq, run:

    make LDFLAGS=-all-static

After make finishes, you'll be able to use `./jq`.  You can also
install it using:

    sudo make install

If you're not using the latest git version but instead building a
released tarball (available on the website), then you won't need to
run `autoreconf` (and shouldn't), and you won't need flex or bison.

To cross-compile for OS X and Windows, see docs/Rakefile's build task
and scripts/crosscompile.  You'll need a cross-compilation environment,
such as Mingw for cross-compiling for Windows.

Cross-compilation requires a clean workspace, then:

    # git clean ...
    autoreconf -i
    ./configure
    make distclean
    scripts/crosscompile <name-of-build> <configure-options>

Use the --host= and --target= ./configure options to select a
cross-compilation environment.  See also the wiki.

Send questions to https://stackoverflow.com/questions/tagged/jq or to the #jq channel (http://irc.lc/freenode/%23jq/) on Freenode (https://webchat.freenode.net/).

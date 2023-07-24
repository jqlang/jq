jq
==

jq is a lightweight and flexible command-line JSON processor.

If you want to learn to use jq, read the documentation at
[https://jqlang.github.io/jq](https://jqlang.github.io/jq).  This
documentation is generated from the docs/ folder of this repository.
You can also try it online at [jqplay.org](https://jqplay.org).

If you want to hack on jq, feel free, but be warned that its internals
are not well-documented at the moment. Bring a hard hat and a
shovel.  Also, read the wiki: https://github.com/jqlang/jq/wiki, where
you will find cookbooks, discussion of advanced topics, internals,
release engineering, and more.

Source tarball and built executable releases can be found on the
homepage and on the github release page, https://github.com/jqlang/jq/releases.
Docker image is available at https://github.com/jqlang/jq/pkgs/container/jq.

If you're building directly from the latest git, you'll need libtool, make, automake,
and autoconf installed.
To get regexp support you'll also need to install Oniguruma or clone it as a
git submodule as per the instructions below. To build, run:

    git submodule update --init # if building from git to get oniguruma
    autoreconf -i               # if building from git
    ./configure --with-oniguruma=builtin
    make -j8
    make check

Developers must `--enable-maintainer-mode` when making changes to the
jq parser and lexer which also requires bison and flex to be installed.

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

Use the `--host=` and `--target=` ./configure options to select a
cross-compilation environment.  See also
["Cross compilation"](https://github.com/jqlang/jq/wiki/Cross-compilation) on
the wiki.

To compile jq to WebAssembly, install the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), then:

    git submodule update --init # if building from git to get oniguruma
    autoreconf -i               # if building from git
    emconfigure ./configure --with-oniguruma=builtin
    emmake make EXEEXT=.js CFLAGS="-O2" LDFLAGS="-s EXPORTED_RUNTIME_METHODS=['callMain']"


# Community

* Send questions to https://stackoverflow.com/questions/tagged/jq.
* Join our [Discord server](https://discord.gg/yg6yjNmgAC).

jq
==

jq is a command-line JSON processor.

If you want to learn to use jq, read the documentation at
[http://stedolan.github.com/jq](http://stedolan.github.com/jq). This
documentation is generated from the docs/ folder of this repository.

If you want to hack on jq, feel free, but be warned that its internals
are not well-documented at the moment. Bring a hard hat and a
shovel. Also, read the wiki: http://github.com/stedolan/jq/wiki

To build jq, run

    ./configure
    make
    sudo make install (optionally)

If you've just checked out the latest version from git (rather than
using a released source tarball) then you'll need to run this first:

    autoreconf

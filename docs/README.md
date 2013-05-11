Documentation
=============

The jq website, manpages and some of the tests are generated from this
directory. The directory holds a [Bonsai](http://tinytree.info)
website, and the manual is a YAML file in `content/3.manual`.

To build the documentation (including building the jq manpage), you'll
need a working Ruby setup. The easiest way to get one is to install
RVM and Ruby 1.9.3 like so:

    \curl -L https://get.rvm.io | bash -s stable --ruby=1.9.3

After that finishes installing, you'll need to make sure RVM is on
your path by doing `source $HOME/.rvm/scripts/rvm`, or just opening a
new shell. See <http://rvm.io> for more info on RVM.

Once RVM is installed, you can install all the dependencies for jq's
documentation build by running this from the `docs` directory:

    bundle install

When bundle manages to install the dependencies, rerun `./configure`
in the jq root directory and then the Makefile will be able to
generate the jq manpage.

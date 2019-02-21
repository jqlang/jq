Documentation
=============

The jq website, manpages and some of the tests are generated from this
directory. The manual is a YAML file in `content/manual`.

To build the documentation (including building the jq manpage), you'll
need python3 and pipenv. You can install pipenv like so:

    pip install pipenv

Though, you may need to say pip3 instead, depending on your system. Once
you have pipenv installed, you can install the dependencies by running
`pipenv install` from the `docs` directory.

Once this is done, rerun `./configure` in the jq root directory and then
the Makefile will be able to generate the jq manpage.

To build the website, run `pipenv run ./build_website.py` from inside
the `docs` directory.

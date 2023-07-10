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

Also, you may need to run `virtualenv -p /usr/bin/python3 venv/` and
then `source venv/bin/activate`, and only then `pipenv install`.

Once this is done, rerun `./configure` in the jq root directory and then
the Makefile will be able to generate the jq manpage.  You can also just
run `pipenv run build_manpage.py` in the `docs` directory to build the
`jq.1` page manually, and `pipenv run build_mantests.py` to build the
contents of `tests/man.test`.

To build the website, run `pipenv run ./build_website.py` from inside
the `docs` directory.

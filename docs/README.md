Documentation
=============

The jq website, manpages and some of the tests are generated from this
directory. The manual is a Markdown file in `content/manual/manual.md`.

To build the jq manpage and the tests contained in it, you'll
need `pandoc` (at least version 3.0).
To build the website, you'll additionally
need `python3` and `pipenv`. You can install `pipenv` like so:

    pip install pipenv

Though, you may need to say `pip3` instead, depending on your system. Once
you have `pipenv` installed, you can install the dependencies by running
`pipenv sync` from the `docs/` directory.

Also, you may need to run `virtualenv -p /usr/bin/python3 venv/` and
then `source venv/bin/activate`, and only then `pipenv sync`.

Once this is done, rerun `./configure --enable-docs` in the jq root directory and then
the `Makefile` will be able to generate the jq manpage.  You can just run
`make jq.1` to build the manpage manually, and `make tests/man.test` to
update the manual tests.

To build the website, run `pipenv run python3 build_website.py --root /output`
in the `docs/` directory. To serve them locally, you can run
`python3 -m http.server`.

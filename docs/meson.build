pipenv = find_program('pipenv', required: false)
if not pipenv.found()
    warning(f'''
        Error checking python dependencies:
        *****************************************************************
        *  Python dependencies for building jq documentation not found. *
        *  You can still build, install and hack on jq, but the manpage *
        *  will not be rebuilt and new manpage tests will not be run.   *
        *  See docs/README.md for how to install the docs dependencies. *
        *****************************************************************
    ''')
    subdir_done()
endif

docs_pipenv = custom_target('docs_pipenv',
    command: [
        'bash', '-c',
        '(cd @0@ && pipenv sync) && touch docs/pipenv.flag'.format(meson.current_source_dir())
    ],
    depend_files: [
        'Pipfile',
        'Pipfile.lock'
    ],
    output: 'pipenv.flag',
)

run_target('build_mantests',
    command: [
        'bash', '-c',
        'cd @0@;'.format(meson.current_source_dir())
        + 'pipenv run ./validate_manual_schema.py content/manual/dev/manual.yml;'
        + 'pipenv run ./build_mantests.py',
    ],
    depends: [docs_pipenv]
)


custom_target('manpage',
    command: [
        'bash', '-c',
        'cd @0@;'.format(meson.current_source_dir())
        + 'pipenv run ./validate_manual_schema.py content/manual/dev/manual.yml;'
        + 'pipenv run ./build_manpage.py',
    ],
    capture: true,
    output: 'jq.1',
    install: true,
    install_dir: get_option('mandir'),
    depends: [docs_pipenv]
)

install_data(
    '../README.md',
    '../AUTHORS',
    '../NEWS.md',
    install_dir: get_option('datadir') / 'doc' / 'jq',
)

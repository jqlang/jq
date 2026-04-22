
tests = [
    'mantest',
    'jqtest',
    'shtest',
    'utf8test',
    'base64test',
    'uritest',
]

if not is_win32
    tests += ['optionaltest']
endif

if oniguruma.found()
    tests += ['onigtest', 'manonigtest']
endif

foreach test: tests
    test(
        test,
        find_program(test),
        env: {
            'JQ': jq.full_path(),
            'ENABLE_VALGRIND': get_option('valgrind') ? '1' : '0',
        }
    )
endforeach

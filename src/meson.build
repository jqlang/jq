
version_file = vcs_tag(
    command: ['../scripts/version'],
    input: 'version.h.in',
    output: 'version.h',
)

builtin_inc = custom_target(
    command: ['../scripts/generate_builtin_inc', '@INPUT@', '@OUTPUT@'],
    input: 'builtin.jq',
    output: 'builtin.inc',
)

# TODO:
config_opts_inc = custom_target(
    command: ['echo', '#define JQ_CONFIG ""'],
    capture: true,
    output: 'config_opts.inc',
)

# Support unavailable flex, bison tools
if flex.found()
    lexer_c_h = custom_target(
        command: [flex, '-o', '@OUTPUT0@', '--header-file=@OUTPUT1@', '@INPUT@'],
        input: 'lexer.l',
        output: ['lexer.c', 'lexer.h'],
    )
else
    lexer_c_h = ['lexer.c', 'lexer.h']
endif

if yacc.found()
    parser_c_h = custom_target(
        command: [yacc, '-o', '@OUTPUT0@', '--header=@OUTPUT1@', '@INPUT@'],
        input: 'parser.y',
        output: ['parser.c', 'parser.h'],
    )
else
    parser_c_h = ['parser.c', 'parser.h']
endif

source_files = [
    'builtin.c',
    'bytecode.c',
    'compile.c',
    'execute.c',
    'jq_test.c',
    'jv.c',
    'jv_alloc.c',
    'jv_aux.c',
    'jv_dtoa.c',
    'jv_dtoa_tsd.c',
    'jv_file.c',
    'jv_parse.c',
    'jv_print.c',
    'jv_unicode.c',
    'linker.c',
    'locfile.c',
    'util.c',
    builtin_inc,
    lexer_c_h,
    parser_c_h,
]

c_args = [
    '-Wno-unused-parameter',
    '-Wno-unused-function',
    '-Wno-pedantic',
]

foreach key : config.keys()
    value = config.get(key)
    if f'@value@' != '0'
        c_args += [f'-D@key@=@value@']
    endif
endforeach

if is_win32
    c_args += ['-municode']
endif


libinject_errors = static_library('inject_errors',
    'inject_errors.c',
    c_args: c_args,
)


libjq = library('jq',
    source_files,
    c_args: c_args,
    include_directories: [
        root_include,
    ],
    dependencies: [
        decnumber,
        math,
        shlwapi,
        oniguruma,
        threads,
    ],
    link_with: [
        get_option('error-injection') ? libinject_errors : [],
    ],
    link_args: [
        # '-Wl,-export-symbols-regex=^j[qv]_',
        # '-Wl,-version-info=1:4:0',
    ],
    install: true,
    version: '1.0.4',
)
# TODO: linker version script

install_headers('jq.h', 'jv.h')

pkgconfig.generate(libjq,
    name: 'libjq',
    url: project_url,
    version: meson.project_version(),
    description: 'Library to process JSON using a query language',
)


jq = executable('jq',
    'main.c',
    version_file,
    config_opts_inc,
    c_args: c_args,
    link_with: [libjq],
    include_directories: [root_include],
    install: true,
)

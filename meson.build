project(
  'jq',
  'c',
  version: run_command(['scripts/version'], check: true).stdout().strip(),
  default_options: ['warning_level=3']
)

meson.add_dist_script(['scripts/version', 'set-dist', meson.project_version()])

project_url = 'https://jqlang.org'

pkgconfig = import('pkgconfig')

config = configuration_data()
cc = meson.get_compiler('c')

##############################################################################
# Check dependencies

flex = find_program(['flex', 'lex'])
yacc = find_program('bison', version: '>=2.3')

if target_machine.endian() == 'big'
    config.set10('IEEE_MC68k', true)
elif target_machine.endian() == 'little'
    config.set10('IEEE_8087', true)
else
    error('unsupported endianness', target_machine.endian())
endif

have_cygwin = cc.has_header('sys/cygwin.h')
have_shlwapi = cc.has_header('shlwapi.h')
# fixme: target_machine.system() == 'windows' ?
is_win32 = have_shlwapi and not have_cygwin
shlwapi = is_win32 ? cc.find_library('shlwapi') : dependency('', required: false)

config.set_quoted('VERSION', meson.project_version())

source_arguments = [
    '-D_ALL_SOURCE=1',
    '-D_DARWIN_C_SOURCE=1',
    '-D_GNU_SOURCE=1',
    '-D_HPUX_ALT_XOPEN_SOCKET_API=1',
    '-D_NETBSD_SOURCE=1',
    '-D_OPENBSD_SOURCE=1',
    '-D_POSIX_PTHREAD_SEMANTICS=1',
    '-D_TANDEM_SOURCE=1',
    '-D__EXTENSIONS__=1',
]
add_project_arguments(source_arguments, language: 'c')

config.set10('USE_DECNUM', get_option('decnum'))

math = cc.find_library('m', required : false)

threads = dependency('threads')
config.set10('HAVE_PTHREAD', threads.found())

have_thread_local = cc.links('''
        static __thread int x ;
        int main () { x = 123; return x; }
    ''',
    name: 'thread local test'
)
config.set10('HAVE___THREAD', have_thread_local)

check_funcs = {
    'memmem': '',
    'isatty': '#include <unistd.h>',
    '_isatty': '#include <io.h>',
    'strptime': '#include <time.h>',
    'strftime': '#include <time.h>',
    'setenv': '#include <stdlib.h>',
    'timegm': '#include <time.h>',
    'gmtime_r': '#include <time.h>',
    'gmtime': '#include <time.h>',
    'localtime_r': '#include <time.h>',
    'localtime': '#include <time.h>',
    'gettimeofday': '#include <sys/time.h>',
    'setlocale': '#include <locale.h>',
    'arc4random': '#include <stdlib.h>',
    'getentropy': '#include <unistd.h>',
    'pthread_key_create': '#include <pthread.h>',
    'pthread_once': '#include <pthread.h>',
    'atexit': '#include <stdlib.h>',
}

check_funcs_math = [
    'acos', 'acosh', 'asin', 'asinh', 'atan2', 'atan', 'atanh', 'cbrt', 'ceil',
    'copysign', 'cos', 'cosh', 'drem', 'erf', 'erfc', 'exp10', 'exp2', 'exp',
    'expm1', 'fabs', 'fdim', 'floor', 'fma', 'fmax', 'fmin', 'fmod', 'frexp',
    'gamma', 'hypot', 'j0', 'j1', 'jn', 'ldexp', 'lgamma', 'log10', 'log1p',
    'log2', 'log', 'logb', 'modf', 'lgamma_r', 'nearbyint', 'nextafter',
    'nexttoward', 'pow', 'remainder', 'rint', 'round', 'scalb', 'scalbln',
    'significand',  'sin', 'sinh', 'sqrt', 'tan', 'tanh', 'tgamma', 'trunc',
    'y0', 'y1', 'yn',
    # scalbn and ilogb are used on macos to replace significand if we don't have frexp
    'scalbn', 'ilogb',
    # macOS has an __exp10
    '__exp10',
]
foreach func : check_funcs_math
    check_funcs = check_funcs + {func: '#include <math.h>'}
endforeach

foreach func, include : check_funcs
    macro = f'HAVE_@0@'.format(func.underscorify().to_upper())
    config.set10(macro, cc.has_function(func, args: source_arguments, prefix: include, dependencies: math))
endforeach

config.set10('HAVE_TM_TM_GMT_OFF', cc.has_member('struct tm', 'tm_gmtoff', prefix: '#include <time.h>'))
config.set10('HAVE_TM___TM_GMT_OFF', cc.has_member('struct tm', '__tm_gmtoff', prefix: '#include <time.h>'))

config.set10('HAVE_ALLOCA_H', cc.has_header('alloca.h'))
config.set10('HAVE_ALLOCA', cc.has_function('alloca', prefix: '#include <alloca.h>'))

check_headers = [
    'dlfcn.h',
    'inttypes.h',
    'stdint.h',
    'stdio.h',
    'stdlib.h',
    'strings.h',
    'string.h',
    'sys/stat.h',
    'sys/types.h',
    'unistd.h',
    'wchar.h',
]

foreach header: check_headers
    macro = f'HAVE_@0@'.format(header.underscorify().to_upper())
    config.set10(macro, cc.has_header(header))
endforeach


root_include = include_directories('.')

if get_option('docs')
    subdir('docs')
endif
subdir('vendor')
subdir('src')
subdir('tests')

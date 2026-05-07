vendor_include = include_directories('.')


decnumber_lib = static_library('decNumber',
    'decNumber/decContext.c',
    'decNumber/decNumber.c',
)

decnumber = declare_dependency(
    link_with: decnumber_lib,
    include_directories: vendor_include,
)


if get_option('oniguruma') == 'system'
    oniguruma = declare_dependency(
        dependencies: dependency('oniguruma'),
        compile_args: ['-DHAVE_LIBONIG'],
    )
elif get_option('oniguruma') == 'builtin'
    cmake = import('cmake')
    oniguruma_opts = cmake.subproject_options()
    oniguruma_opts.add_cmake_defines({'BUILD_SHARED_LIBS': false})
    oniguruma_opts.add_cmake_defines({'CMAKE_POSITION_INDEPENDENT_CODE': true})
    oniguruma_opts.append_compile_args('c', '-w')
    oniguruma_proj = cmake.subproject(
        'oniguruma',
        options: oniguruma_opts,
    )

    oniguruma_dep = oniguruma_proj.dependency('onig')
    oniguruma = declare_dependency(
        dependencies: oniguruma_dep,
        compile_args: ['-DHAVE_LIBONIG'],
    )
else
    oniguruma = dependency('', required: false)
endif

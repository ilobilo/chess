project('chess', 'cpp',
    version : 'v0.1',
    default_options : ['cpp_std=c++20']
)

sources = [
    'src/main.cpp',
    'src/chess.cpp'
]

includes = include_directories('src')

frozen_dep = import('cmake').subproject('frozen').dependency('frozen')
conflict_dep = dependency('conflict')
fmt_dep = dependency('fmt')

executable('chess',
    dependencies : [frozen_dep, conflict_dep, fmt_dep],
    sources : sources,
    include_directories : includes
)
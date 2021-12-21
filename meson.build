project('katan', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

rl_dep = dependency('raylib')

executable('katan',
           'katan.c',
           dependencies: [rl_dep],
           install : true)

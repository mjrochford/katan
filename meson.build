project('katan', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

rl_dep = dependency('raylib')

sources = ['rules.c', 'katan.c']

tests = executable('tests', sources: ['tests.c', 'rules.c'], dependencies: [rl_dep])
test('rule test', tests)

executable('katan',
           sources: sources,
           dependencies: [rl_dep],
           install : true)

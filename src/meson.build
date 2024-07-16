qt6 = import('qt6')
lupdate = find_program('lupdate')

qobject_headers = [
  'win',
  'logs',
  'options',
  'process',
]
srcs = [ 'main.cpp' ]
deps = [ qt6_dep ]
headers = []

foreach name : qobject_headers
  srcs += name + '.cpp'
  headers += name + '.h'
endforeach

languages = [ 'en', 'id' ]
ts_files = ['aria2tray.ts']
ts_files_full = ['translations/aria2tray.ts']


foreach lang : languages
  ts_file = 'aria2tray_' + lang + '.ts'
  ts_files += ts_file
  ts_files_full += 'translations/' + ts_file
endforeach

# update translations
lup_out = run_command(lupdate, srcs, '-ts', ts_files_full, check: true)
message(lup_out.stdout().strip())

subdir('translations')
moc_files = qt6.compile_moc(headers: headers, dependencies: qt6_dep)

executable('aria2tray', srcs, moc_files, dependencies: deps)

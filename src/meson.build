qt6 = import('qt6')
lupdate = find_program('lupdate')

qobject_headers = [
  'win',
  'logs',
  'options',
  'process',

  # websocket ipc
  'ipc/wsclient',
  'ipc/wsserver',
]
srcs = [ 'main.cpp', 'ipc/jsonrpc.cpp', 'ipc/jsonrpc.h' ]
  
deps = [ qt6_dep ]
headers = []

foreach name : qobject_headers
  srcs += name + '.cpp'
  headers += name + '.h'
endforeach

# localization
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


if target_machine.system() == 'windows'
  win = import('windows')
  win_res =  win.compile_resources('../resource.rc', depend_files: '../assets/icon.ico')
  executable('aria2tray', srcs, moc_files, win_res, dependencies: deps, win_subsystem: 'windows')
else
  executable('aria2tray', srcs, moc_files, dependencies: deps)
endif


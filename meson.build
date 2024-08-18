project('aria2tray','cpp', default_options: ['cpp_std=c++17'], license: 'MIT', license_files: 'LICENSE')
  
add_global_arguments('-DA2T_IPC_PORT=31795', language: 'cpp')
add_global_arguments('-DA2T_VERSION="0.1.0"', language: 'cpp')

qt6_modules = ['Widgets', 'Core', 'Gui', 'WebSockets']
if target_machine.system() == 'linux'
  add_global_arguments('-DA2T_DATA_DIR="/usr/share/aria2tray"', language: 'cpp')
elif target_machine.system() == 'windows'
  qt6_modules += 'EntryPoint'
endif

qt6_dep = dependency('qt6', modules: qt6_modules)

subdir('src')

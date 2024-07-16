## Dependencies

- Qt6 >= 6.5
- Meson
- Ninja

## Building

Linux build:

1. run `make` or `make release` and binary is ready inside `build/linux/src` folder

Windows build:

1. Download qt source from [Qt website](https://download.qt.io/official_releases/qt/) and compile only qtbase and qttools.
2. Add Qt bin folder to `PATH` by editing `QT_BIN` variable in `build.bat`
3. execute `build.bat` and executable is produced inside `build/win/src` folder

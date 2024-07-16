@echo off

set QT_BIN=%cd%\..\qt-src\qtbase\bin
set PATH=%PATH%;%QT_BIN%;%QT_TOOLS%

set builddir=build
set win_builddir=%builddir%\win

meson setup %win_builddir% --buildtype=release
ninja -C %win_builddir%

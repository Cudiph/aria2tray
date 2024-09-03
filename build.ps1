$QT_BIN=";$($PWD.path)/../qt-src/qtbase/bin"
$old_path = $env:PATH
$env:PATH += $QT_BIN

$outdir="build/win"
$meson_flags=""

# resource compile
rc resource.rc
cvtres /MACHINE:x64 .\resource.res


if ($args[0] -eq "release") {
  $outdir="dist/win"
  $meson_flags="--buildtype=release"
} elseif ($args[0] -eq "clean") {
  if ($args[1] -eq "release") {
    rm dist/win
  } else {
    rm build/win
  }
  exit
}

meson setup $outdir $meson_flags.split()
ninja -C $outdir

if ($args[0] -eq "release") {
  windeployqt.exe --release $outdir/src/aria2tray.exe
}

$env:PATH = $old_PATH

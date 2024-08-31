_pkgname=aria2tray
pkgname="$_pkgname-git"
pkgver=0.0.1
pkgrel=1
pkgdesc='Qt frontend for running aria2 in the background'
arch=('x86_64')
url="https://github.com/Cudiph/$_pkgname"
license=('MIT')
depends=('qt6-base' 'aria2' 'qt6-websockets')
make_depends=('qt6-tools')
opt_depends=('qt6-wayland: wayland support')
source=("git+$url.git")
md5sums=('SKIP')

build() {
	cd "$_pkgname"
	make release
}

package() {
	cd "$_pkgname"
	make DESTDIR="$pkgdir" install
}

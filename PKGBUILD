# Maintainer: seja-arcticfox <seja.czstudio@gmail.com>
pkgname=vidcom
pkgver=0.82
pkgrel=1
pkgdesc="Archive your videos"
arch=('x86_64')
url="https://github.com/seja-arctic-fox/vidcom"
license=('GPL-3.0-only')
depends=('gtkmm-4.0' 'libadwaita' 'jsoncpp' 'ffmpeg')
makedepends=('meson' 'ninja')
changelog=
source=("$pkgname-$pkgver.tar.gz::https://github.com/seja-arctic-fox/vidcom/archive/refs/tags/$pkgver.tar.gz")
sha256sums=('9a4f17208897d14d06c82339900ed2f34a980dd4e926f1838adffc686e044d71')

build() {
	cd "$pkgname-$pkgver"
	meson setup build --prefix=/usr
	meson compile -C build
}

package() {
	cd "$pkgname-$pkgver"
	DESTDIR="$pkgdir" meson install -C build
}

# Maintainer: seja-arcticfox <seja.czstudio@gmail.com>
pkgname=vidcom
pkgver=0.83
pkgrel=1
pkgdesc="Archive your videos"
arch=('x86_64')
url="https://github.com/seja-arctic-fox/vidcom"
license=('GPL-3.0-only')
depends=('gtkmm-4.0' 'libadwaita' 'jsoncpp' 'ffmpeg')
makedepends=('meson' 'ninja')

build() {
    meson setup "$startdir/build" "$startdir" --prefix=/usr
	meson compile -C "$startdir/build"
}

package() {
	DESTDIR="$pkgdir" meson install -C "$startdir/build"
}

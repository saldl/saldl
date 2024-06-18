#!/bin/bash

AUR_PKGS=(
  mingw-w64-environment
  mingw-w64-pkg-config

  mingw-w64-configure
  mingw-w64-meson
  mingw-w64-cmake

  mingw-w64-libiconv

  mingw-w64-libunistring

  mingw-w64-zlib
  mingw-w64-libidn2

  mingw-w64-openssl

  mingw-w64-zstd
  mingw-w64-brotli
  mingw-w64-libnghttp2
  mingw-w64-libpsl
  mingw-w64-libssh2

  mingw-w64-curl
)

pexec () {
        [ $NO_P_ECHO ] || echo "$@" >&2
        [ $NO_P_EXEC ] || "$@"
}

# build aur deps
for pkg in ${AUR_PKGS[@]}; do
  pwd
  pexec git clone https://aur.archlinux.org/${pkg}.git $pkg
  pushd $pkg
  pwd
  pexec makepkg -s --nocheck --noconfirm
  popd
done

pwd

# build our libevent
pushd mingw-w64-libevent-saldl
pwd
pexec makepkg -s --nocheck --noconfirm

popd
pwd

pexec pacman -U --noconfirm mingw-w64-*/mingw-w64-*.pkg.tar.*

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
  #mingw-w64-libssh2
)

pexec () {
        [ $NO_P_ECHO ] || echo "$@" >&2
        [ $NO_P_EXEC ] || "$@"
}

pexec useradd -mU -s /bin/bash user
echo 'allow user to run pacman via sudo'
echo 'user  ALL=(ALL) NOPASSWD: /usr/bin/pacman' >> /etc/sudoers
su user

# build aur deps
for pkg in ${AUR_PKGS[@]} mingw-w64-curl mingw-w64-libevent-saldl; do
  pwd
  if ((${AUR_PKGS[(I)$pkg]})); then
    pexec git clone https://aur.archlinux.org/${pkg}.git $pkg
  fi
  pushd $pkg
  if [[ $pkg == "mingw-w64-libpsl" ]]; then
    sed -i 's|-meson |&--default-library=both |' PKGBUILD
  elif [[ $pkg == "mingw-w64-zstd" ]]; then
    sed -i 's|STATIC=OFF|STATIC=ON|' PKGBUILD
  #elif [[ $pkg == "mingw-w64-curl" ]]; then
  #  sed -i 's|openssl|mbedtls|' PKGBUILD
  #  sed -i -e "s|'mingw-w64-libssh2'||" -e 's|--with-libssh2||' PKGBUILD
  fi
  pwd
  pexec chown -R user:root .
  pexec su user -c 'makepkg -s --nocheck --noconfirm --skippgpcheck'
  pexec pacman -U --noconfirm mingw-w64-*.pkg.*
  rm -rf src pkg
  popd
done

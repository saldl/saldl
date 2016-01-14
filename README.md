# ![saldl banner](https://raw.githubusercontent.com/saldl/misc/master/saldl_banner_alpha.png)

A multi-platform command-line downloader optimized for speed and early
preview, based on libcurl.

**saldl** splits a download into fixed-sized chunks and download
them in-order with multiple concurrent connections. Classic modes
(single chunk single connection, no. of chunks = no. of connections)
are also available as *options*.

More than 57 command-line options are available to customize saldl's behavior.

## Documentation

Detailed documentation is available in the form of
a [manual page](https://saldl.github.io/saldl.1.html).

## Contact

[Users Chat & Questions](https://github.com/saldl/saldl/issues/4)

## FlashGot integration

 Check out the `README` and the scripts in the [flashgot](flashgot/)
 directory.

## Screenshot

```
   -s4m => set chunk_size to 4 MiB.
   -c4  => use 4 concurrent connections.
   -l4  => download the last 4 chunks first.
```

![saldl screenshot](https://raw.githubusercontent.com/saldl/misc/master/saldl.png)

## Dependencies

 * **Runtime Dependencies**

  * [libcurl](https://github.com/bagder/curl) >= 7.42
  * [libevent + libevent_pthreads](https://github.com/libevent/libevent) >= 2.0.20

 * **Build Dependencies**

  * GCC or Clang
  * python (for waf)

 * **Optional Build Dependencies**

  * git (to get the current version).
  * asciidoc (to build the manual).

  **Note**: `docbook-xsl` should be installed too if it's not already
            a dependency of `asciidoc`. While this is theoritically not
            necessary, falling back to getting XSL stylesheets directly
            from SourceForge seems to be unreliable nowadays.

## Build

  **saldl** has been tested on GNU/Linux, NetBSD, FreeBSD, OpenBSD, Mac OSX, and Windows.

  **saldl** should build and run successfully on all POSIX platforms.
  If anyone runs into any build failures, reporting them would be highly
  appreciated.

### POSIX

 * **General**

  Run `./waf --help` first and check out the available options.

  A typical example would be:

  ```
  ./waf configure --prefix=/usr

  ./waf

  ./waf install --destdir=${pkgdir}
  ```

 * **Arch Linux**

  **saldl** is available in the [AUR](https://aur.archlinux.org/packages/saldl-git).

 * **Mac OSX**

  **saldl** can be installed with [Homebrew](http://brew.sh).

  For the latest release, run:

      brew install saldl

  For the latest GIT head, run:

      brew install --HEAD saldl

  **Note**: saldl will be linked against system `libcurl` if the Mac OSX
            version installed is Yosemite (10.10) or newer. Pass `--with-curl`
            if you want to link against the latest `libcurl` release provided
            by Homebrew.

### Windows

 * **MSYS2/Cygwin**

  A [PKGBUILD](MSYS2/PKGBUILD) for MSYS2.

  **Note**: Building against the MSYS2 runtime (or Cygwin's) is
            necessary for **saldl** to work correctly in
            [mintty](https://github.com/mintty/mintty).

 * **MinGW-w64/Native**

  Experimental binaries are now available in
  the [releases](https://github.com/saldl/saldl/releases) page.

  **saldl** requires a terminal emulator with support for ANSI/VT100
  escape sequences (e.g. [ConEmu](https://github.com/Maximus5/ConEmu)
  or [ansicon](https://github.com/adoxa/ansicon)).

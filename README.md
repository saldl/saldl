# saldl

A multi-platform command-line downloader optimized for speed and early
preview, based on libcurl.

**saldl** splits a download into fixed-sized chunks and download
them in-order with multiple concurrent connections. Classic modes
(single chunk single connection, no. of chunks = no. of connections)
are also available as *options*.

Many **options** are available to change the default behavior and
customize network requests.

## Documentation

Detailed documentation is available in the form of a [manual page](https://saldl.github.io/saldl.1.html).

### Where can (potential) users ask questions about saldl?

We consider all questions ,potential users might have about saldl, bugs
in documentation.

Feel free to ask any question about saldl
[here](https://github.com/saldl/saldl/issues/4).

## Screenshot

```
   -s4m => set chunk_size to 4 MiB.
   -c4  => use 4 concurrent connections.
   -l4  => download the last 4 chunks first.
```

![saldl screenshot](https://raw.githubusercontent.com/saldl/misc/master/saldl.png)

## Dependencies

* [libcurl](https://github.com/bagder/curl) >= 7.42
* [libevent + libevent_pthreads](https://github.com/libevent/libevent) >= 2.0.20

## Build Dependencies

* GCC or Clang
* python (for waf)

### Optional build dependencies

* git (to get the current version).
* asciidoc (to build the manual).

## Build

### POSIX

Run `./waf --help` first and check out the available options.

A typical example would be:

```
./waf configure --prefix=/usr

./waf

./waf install --destdir=${pkgdir}
```

#### Arch Linux

saldl is available in the AUR:

https://aur.archlinux.org/packages/saldl-git

### MinGW/Windows

Experimental binaries are now available in
the [releases](https://github.com/saldl/saldl/releases) page.

**saldl** requires a terminal emulater with support for ANSI/VT100
escape sequences.

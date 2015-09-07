# saldl

A command-line downloader with unique features based on libcurl.

By default, saldl splits a download into fixed-sized chunks and download
them in-order with multiple concurrent connections.

Many options are available. They are documented in the [manual page](https://saldl.github.io/saldl.1.html).

# Screenshot

```
   -s4m => set chunk_size to 4 MiB.
   -c4  => use 4 concurrent connections.
   -l4  => download the last 4 chunks first.
```

![saldl screenshot](https://raw.githubusercontent.com/saldl/misc/master/saldl.png)

# Dependencies

* libcurl
* libevent(with pthreads)

# Build Dependencies

* git
* python (for waf)
* asciidoc

# Build

./waf configure --prefix=PREFIX [ --bindir=BINDIR --mandir=MANDIR  --disabla-man ]

./waf

./waf install --destdir=DESTDIR

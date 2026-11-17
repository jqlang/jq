# jq

`jq` is a lightweight and flexible command-line JSON processor akin to `sed`,`awk`,`grep`, and friends for JSON data. It's written in portable C and has zero runtime dependencies, allowing you to easily slice, filter, map, and transform structured data.

## Documentation

- **Official Documentation**: [jqlang.org](https://jqlang.org)
- **Try jq Online**: [play.jqlang.org](https://play.jqlang.org)

## Installation

### Prebuilt Binaries

Download the latest releases from the [GitHub release page](https://github.com/jqlang/jq/releases).
The platform-specific assets are standalone executables, not archives. On macOS and Linux,
rename the downloaded file to `jq` and make it executable with `chmod +x jq`.

### Docker Image

Pull the [jq image](https://github.com/jqlang/jq/pkgs/container/jq) to start quickly with Docker.

#### Run with Docker

##### Example: Extracting the version from a `package.json` file

```bash
docker run --rm -i ghcr.io/jqlang/jq:latest < package.json '.version'
```

##### Example: Extracting the version from a `package.json` file with a mounted volume

```bash
docker run --rm -i -v "$PWD:$PWD" -w "$PWD" ghcr.io/jqlang/jq:latest '.version' package.json
```

### Building from source

#### Dependencies

- libtool
- make
- automake
- autoconf

#### Instructions

```console
git submodule update --init    # if building from git to get oniguruma
autoreconf -i                  # if building from git
./configure --with-oniguruma=builtin
make clean                     # if upgrading from a version previously built from source
make -j8
make check
sudo make install
sudo ldconfig                  # on Linux, if jq cannot find libjq.so
```

Build a statically linked version:

```console
make clean
make LDFLAGS=-all-static
```

If you're not using the latest git version but instead building a released tarball (available on the release page), skip the `autoreconf` step, and flex or bison won't be needed.

##### Faster JSON parsing with yyjson

jq bundles [yyjson](https://github.com/ibireme/yyjson) in `vendor/yyjson/` and
uses it as a fast path for parsing JSON. There is nothing to enable: it is
compiled in by the normal build above and is always on. It is used for:

- regular files given as arguments, or stdin redirected from a file
  (`jq . file.json`, `jq . < file.json`)
- `fromjson` and `--argjson`/`--jsonargs` text

It is not used for pipes or terminals (`curl ... | jq`), `--stream`, `--seq`,
or `--raw-input`; those use jq's original parser. If yyjson rejects any input
(NaN, invalid UTF-8, very deep nesting, syntax errors), jq re-parses it with
the original parser, so results and error messages are the same as before.

To try it, run jq on a large file and compare against a jq without it:

```console
time ./jq -c . big.json > /dev/null
time /usr/bin/jq -c . big.json > /dev/null
```

##### Cross-Compilation

For details on cross-compilation, check out the [GitHub Actions file](.github/workflows/ci.yml) and the [cross-compilation wiki page](https://github.com/jqlang/jq/wiki/Cross-compilation).

## Community & Support

- Questions & Help: [Stack Overflow (jq tag)](https://stackoverflow.com/questions/tagged/jq)
- Chat & Community: [Join us on Discord](https://discord.gg/yg6yjNmgAC)
- Wiki & Advanced Topics: [Explore the Wiki](https://github.com/jqlang/jq/wiki)

## License

`jq` is released under the [MIT License](COPYING). `jq`'s documentation is
licensed under the [Creative Commons CC BY 3.0](COPYING).
`jq` uses parts of the open source C library "decNumber", which is distributed
under [ICU License](COPYING)

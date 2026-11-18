# jq

`jq` is a lightweight and flexible command-line JSON processor akin to `sed`,`awk`,`grep`, and friends for JSON data. It's written in portable C and has zero runtime dependencies, allowing you to easily slice, filter, map, and transform structured data.

## About this fork

This fork speeds up jq's JSON input and output:

- **[yyjson](https://github.com/ibireme/yyjson)** parses JSON input (files,
  stdin/pipes, `fromjson`, `--argjson`). Anything it does not accept is handed
  to jq's original parser, so results and error messages are unchanged.
- **[stb_sprintf](https://github.com/nothings/stb_sprintf)** formats the
  `\uXXXX` escapes when printing, and output is written with unlocked stdio
  calls on glibc. Most of the print-side gain comes from the unlocked writes;
  stb_sprintf itself contributes very little.

Both are git submodules (`git submodule update --init`) and are always on; see
[Faster JSON parsing with yyjson](#faster-json-parsing-with-yyjson) for details.
Build and install with `./install.sh`.

### Benchmarks

Compared with the distribution's jq 1.8.1 on the same machine. Output was
verified identical (md5 of the result). Times are wall-clock; the first two
rows are the mean of two runs. Your numbers will vary with hardware, data and
whatever else is using the machine.

| Input | Command | jq 1.8.1 | this fork |
|---|---|---|---|
| 1.35 GB `compile_commands.json` (mostly strings) | `jq -c . file` | 11.1 s | 5.2 s |
| same, from a pipe | `cat file \| jq -c .` | 11.4 s | 5.4 s |
| same, parse only | `jq length file` | 9.6 s | 3.7 s |
| same, with a filter | `jq '[.[]\|select(.file\|endswith(".cpp"))]\|length' file` | 9.4 s | 3.8 s |
| 56 MB generated array of objects | `jq -c . file` | 1.55 s | 0.92 s |

What contributes: yyjson parsing, memory-mapping the input file (one working
copy instead of two), sharing repeated object keys, not freeing the last parsed
input at exit, plain-double numbers where that is lossless, a 256 KiB stdout
buffer, unlocked stdio writes, and `-O3 -flto` (about 5%). Gains are larger for
string-heavy files.

Trade-offs:

- **Memory.** The 1.35 GB file peaks at about 5.9 GB resident (stock jq: 2.8 GB)
  because the file, yyjson's tree and jq's values coexist. If you are short on
  memory, run with `JQ_NO_YYJSON=1` to use the original streaming parser.
- Input is read fully before parsing, so a pipe that never closes
  (`tail -f | jq .`) prints nothing until it ends.

To reproduce:

```console
time jq -c . big.json | md5sum           # this fork
time /usr/bin/jq -c . big.json | md5sum  # stock jq, outputs should match
JQ_NO_YYJSON=1 jq -c . big.json          # this fork with the fast path off
```

New to jq? See [jQ_ELI5.md](jQ_ELI5.md) for a list of examples.

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

The quick way, from a git checkout (fetches submodules, builds, tests and
installs; sudo is used only for the install step):

```console
./install.sh                  # or: ./install.sh --prefix ~/.local --no-check
```

By default `jq` is linked statically against its own `libjq`, so an installed
`jq` never picks up a different `libjq.so` from the system. The manual steps:

```console
git submodule update --init    # if building from git to get oniguruma, yyjson and stb_sprintf
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

jq includes [yyjson](https://github.com/ibireme/yyjson) as a git submodule in
`vendor/yyjson/` (run `git submodule update --init` after cloning) and uses it as a fast path for parsing JSON. There is nothing to enable: it is
compiled in by the normal build above and is on by default (set `JQ_NO_YYJSON=1` to turn it off at run time). It is used for:

- files given as arguments, and stdin, whether a regular file or a pipe
  (`jq . file.json`, `jq . < file.json`, `curl ... | jq .`)
- `fromjson` and `--argjson`/`--jsonargs` text

Input is read completely before parsing, so a pipe that never closes
(`tail -f log | jq .`) produces no output until it ends. Not covered:
`--raw-input` (not JSON), `--stream` and `--seq`; those use jq's original
parser. If yyjson rejects any input (NaN, invalid UTF-8, very deep nesting,
syntax errors), jq replays it through the original parser, so results and
error messages are the same as before.

Output is also written with unlocked stdio calls on glibc, and the `\uXXXX`
escapes use [stb_sprintf](https://github.com/nothings/stb_sprintf)
(submodule in `vendor/stb_sprintf/`).

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

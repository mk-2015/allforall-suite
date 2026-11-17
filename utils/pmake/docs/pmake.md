# PMAKE

PMAKE is a small package-building utility for C++ projects. It reads a TOML configuration, resolves source packages, builds each project according to its declared build system, and can generate an installation manifest for the final package.

## Command-line reference

```bash
pmake [options]
```

Available flags:

- `-p, --config <path>`: Path to the TOML configuration file. Default: `PMAKE.TML`.
- `-j, --jobs <n>`: Number of concurrent build jobs.
- `-s, --syncdeps`: Install missing dependencies before building.
- `-i, --install`: Install the package after a successful build.
- `-c, --clean`: Remove build artifacts and work directories.
- `-f, --force`: Force installation or overwrite existing package data.
- `-g, --geninteg`: Generate source integrity checksums.
- `-S, --source`: Extract source files only; do not compile.
- `-u, --uninstall <path>`: Path to an install manifest to uninstall.
- `-V, --version`: Print the version string and exit.
- `-h, --help`: Display CLI help and exit.

Example:

```bash
pmake -p PMAKE.TML -j 8 -i
```

## Configuration file reference

PMAKE expects a TOML file defining the package metadata and at least one source entry. The minimum structure looks like this:

```toml
[package]
name = "example-pkg"
version = "1.0.0"
release = 1
description = "Example package for PMAKE"
architecture = "x86_64"
license = ["MIT"]

[build]
jobs = 4
directory = "build"
output = "pkg"

[build.source1]
name = "library-a"
version = "1.0.0"
source = "https://example.com/library-a.tar.gz"
main = false
build_system = "make"

[build.source2]
name = "example"
version = "1.0.0"
source = "https://example.com/example.tar.gz"
main = true
build_system = "make"
```

### Package table

The `[package]` table defines installation metadata:

- `name`: required package name
- `version`: required package version
- `release`: required positive integer release number
- `description`: optional package summary
- `architecture`: optional target architecture, default `x86_64`
- `license`: optional string or list of identifiers

### Build table

The `[build]` table controls the build job and output layout:

- `jobs`: positive integer, default `4`
- `directory`: working build directory, default `build`
- `output`: output package name, default `pkg`

Each source is declared as a numbered TOML table such as `[build.source1]`, `[build.source2]` and so on. Every source must include:

- `name`
- `version`
- `source`
- `main` (boolean)

The `build_system` field may be one of:

- `make`
- `cmake`
- `meson`
- `autotools`
- `custom`

If `build_system = "custom"`, the source must also define `custom_build`.

## Build workflow

1. Parse and validate the TOML configuration.
2. Ensure exactly one source is marked `main = true`.
3. Download or extract each source entry.
4. Build secondary sources first, then the main package.
5. Optionally install the package when `-i` is used.
6. Generate a manifest for uninstalling or auditing installed content.

## Typical usage

```bash
pmake
pmake -j 8 -i
pmake -p custom/PMAKE.TML -S
pmake -u installed.pkg.manifest
```

## Notes

PMAKE is designed for straightforward package assembly and installation workflows. It is intentionally small and opinionated, so projects should provide a clear TOML definition for build and install behavior.

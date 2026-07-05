# snipe
Snipe is a decentralized package manager for installing, managing, and running packages from simple repository URLs. It is designed to be lightweight, portable, and easy to host.

## Commands

### init
Initializes the local Snipe installation directory and creates the default repository list and package/cache folders.

- command: `snipe init`

### list
Lists the packages currently installed under the local Snipe packages directory.

- command: `snipe list`

### install
Downloads a package archive from a configured repository, extracts it, installs dependencies from `.deps`, and runs `setup.py`.

- command: `snipe install <package-name> <version>`
- latest versions are supported via `latest` or `LATEST`, which makes Snipe request the repository's `.latest` file first

Example:
- `snipe install goop 1.2.3`
- `snipe install goop latest`

### remove
Removes an installed package directory and deletes its cached archive.

- command: `snipe remove <package-name> <version>`

### exec
Runs a tool from an installed package directory.

- command: `snipe exec <package-name> <version> <tool-name> [args...]`

### help
Shows the available commands and usage.

- command: `snipe help`

## Repository format
Each repository entry in `~/.snipe/packrepo.lst` should point to a base URL. Snipe will try the package URL pattern:

- `<repo>/<package-name>/<version>/<package-name>-<version>.tar.gz`
- or, for latest versions, `<repo>/<package-name>/.latest`

Example repository line:
- `https://repo.example.com/pack/`

## Installation layout
The local Snipe installation layout looks like this:

- `~/.snipe/`
  - `cache/`
  - `packages/`
  - `packrepo.lst`
  - `version`

## Package format
Each installed package is stored in a directory named like:

- `~/.snipe/packages/package-name-version`

A package may include:

- `bin/` for executables
- `include/` for headers
- `lib/` for libraries
- `share/` for shared assets
- `.deps` for dependency declarations
- `setup.py` for installation logic
- `.short_description` and `.long_description` for metadata

## Dependency file format
A package can declare dependencies using a `.deps` file with this format:

```deps
https://repo1.tld/ -- pkg-a -- 1.0.0
https://repo2.tld/ -- pkg-b -- latest
```

Snipe reads each dependency entry, installs it recursively, and skips circular dependency loops while a package is already being installed.

## Packaging notes
- Packages should be distributed as `.tar.gz` archives.
- The archive contents are extracted into the package directory.
- After extraction, Snipe runs `setup.py install` from the extracted package directory when a `setup.py` file is present.
- If a package has no `setup.py`, the install is still treated as successful unless a setup step fails.
- package gets 'PKG' Environment varible

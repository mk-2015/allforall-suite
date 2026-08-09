# hpm (Hypr Package Manager) Documentation 
**hpm** is a fast, modern C++ decentralized package manager built for `allforall-suite`. It manages, builds, installs, and searches packages across configured repository endpoints using YAML manifests, SQLite database tracking, and regular expression searching. 

## 1. CLI Usage & Flags 
`hpm [-i|-l|-r|-rf|-sr|-lr|init] [args...]` 
* `init`: Initializes local setup (`~/.hpm` directory, SQLite DB, default config). 
* `-i`: Installs target package(s) using `package:version` (defaults to `latest`). 
* `-r`: Safely removes installed package(s). 
* `-rf`: Forcibly removes package(s). 
* `-l`: Lists all currently installed packages registered in SQLite. 
* `-lr`: Lists all configured repository endpoints from `~/.hpm/.config.yml`. 
* `-sr`, `--search-repo`: Searches remote repositories using regex. 

### Examples 
* `hpm init` 
* `hpm -i goop:1.2.3 pkg-a` 
* `hpm -r goop` 
* `hpm -rf broken-pkg` 
* `hpm -l` * `hpm -lr` 
* `hpm -sr "c\+\+-regex-.*"`

 ## 2. Directory & Database Layout 
`hpm` maintains state using a local SQLite database and YAML configuration: 
* `~/.hpm/db.sqlite3`: Tracks `installed_packages` and `package_files`. 
* `~/.hpm/.config.yml`: Lists repository URLs. 
* `~/.hpm/cache/`: Temporary downloaded archives and cached manifests. 

### Repository Config (`~/.hpm/.config.yml`) 
```yaml
repos: 
- https://repo.example.com/hpm 
- https://custom.repo.org/packages 
```

## 3. Package Manifest Specification (`box.yaml`) 
Repositories host package specs at `//`: 

```yaml
name: "goop" 
version: "1.2.3" 
Dependencies:
 - name: "libdot3" constraint: ">=1.0.0" 

packages: 
 - name: "source" 
   url: "https://repo.example.com/tarballs/goop-1.2.3.zip" 
commands: 
 - "make -j4" 
 - "make install PREFIX=/usr/local" 
```

## 4. Security & Input Sanitization 
To prevent path traversal (`../`) and URL/command injection attacks, `hpm` strictly validates all package names and version strings against `[a-zA-Z0-9._+-]`. Invalid or suspicious characters cause execution to instantly abort safely.

## 5. Security & Keyring Management
`hpm` uses native OpenSSL Ed25519 public-key cryptography to verify package archive signatures (`.sig`) before extraction or execution.

### Keyring CLI Commands
* `hpm key add <name> <pubkey_path>`: Imports a maintainer's public key into `~/.hpm/keyring/`.
* `hpm key remove <name>`: Removes a trusted maintainer's key.
* `hpm key list`: Lists all trusted maintainers in the local keyring.
* `hpm key generate <name> <email> <path>`: Generates a new Ed25519 keypair.
* `hpm key extract <name> <path>`: Exports a stored public key.
* `hpm key sign <privkey> <target_file> <output_sig>`: Generates a signature file for package maintainers.

### Examples
* `hpm key generate maintainer-joe joe@example.com ./joe.priv`
* `hpm key sign ./joe.priv ./my-pkg-1.0.0.zip ./my-pkg-1.0.0.zip.sig`
* `hpm key add official ./official.sig.pub`
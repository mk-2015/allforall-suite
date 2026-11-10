# hostdescv

`hostdescv` is an IPv4/IPv6 host discovery tool built on Boost.Asio that concurrently probes target IP addresses or CIDR blocks across configurable ports and outputs results in text, JSON, TOML, or YAML formats.

## Usage

```bash
hostdescv [OPTIONS] <CIDR or IP>
```

## Options

- `-p`, `--ports PORTS` : Comma-separated ports to probe (default: `80,443,22`).
- `-w`, `--timeout SEC` : Connection timeout in seconds (default: `1.0`).
- `-o`, `--format FMT` : Output format (`text`, `json`, `toml`, `yaml`, default: `text`).
- `-h`, `--help` : Display help message and exit.

## Examples

Scan a local subnet (/24):

```bash
hostdescv 192.168.1.0/24
```

Output results in JSON format:

```bash
hostdescv -o json 10.0.0.0/28
```

Scan specific ports with YAML output:

```bash
hostdescv -p 80,443 -o yaml 192.168.1.50
```

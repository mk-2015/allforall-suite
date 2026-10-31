
# nprobe-tcp

`nprobe-tcp` is a TCP connectivity probing utility built on Boost.Asio, designed for testing TCP port reachability, measuring latency, and performing connection diagnostics.

## Usage

```bash
nprobe-tcp [OPTIONS] HOST PORT
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-t COUNT` : Number of connection attempts (default: `1`).
- `-w TIME` : Connection timeout in seconds (default: `3.0`).
- `-i INTERVAL` : Delay between connection attempts in milliseconds.
- `-o` : Enable machine-readable output format.

## Examples

Probe a TCP port once with default timeout:

```bash
nprobe-tcp example.com 80
```

Probe a TCP port 5 times with a 2-second timeout and 1000ms delay between attempts:

```bash
nprobe-tcp -t 5 -w 2.0 -i 1000 192.168.1.1 443
```

Probe with machine-readable output format:

```bash
nprobe-tcp -o example.com 443
```

## Exit Status

- `0`: Success (all or at least successful connection depending on attempts).
- Non-zero: Connection failure, timeout, or argument error.

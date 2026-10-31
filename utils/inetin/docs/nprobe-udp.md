# nprobe-udp

`nprobe-udp` is a UDP connectivity probing utility built on Boost.Asio, designed for testing UDP port reachability, measuring response latency, and performing UDP diagnostics.

## Usage

```bash
nprobe-udp [OPTIONS] HOST PORT
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-t COUNT` : Number of probes (default: `1`).
- `-w TIME` : Response timeout in seconds (default: `3.0`).
- `-i INTERVAL` : Delay between probes in milliseconds.
- `-o` : Enable machine-readable output format.

## Examples

Probe a UDP port once with default timeout:
```bash
nprobe-udp example.com 53
```

Probe a UDP port 3 times with a 2-second timeout and 500ms delay between probes:
```bash
nprobe-udp -t 3 -w 2.0 -i 500 192.168.1.1 123
```

Probe with machine-readable output format:
```bash
nprobe-udp -o example.com 53
```

## Exit Status

- `0`: Success.
- Non-zero: Probe failure, timeout, or argument error.

# ipcacl

`ipcacl` is an IP subnet calculator utility that computes subnet details including network address, broadcast address, netmask, wildcard mask, usable host range, and host counts from an IPv4 address and CIDR prefix or netmask.

## Usage

```bash
ipcacl [OPTIONS] <IP>[/CIDR]
ipcacl [OPTIONS] <IP> <NETMASK>
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-b`, `--binary` : Show binary representations of IP and netmask.
- `-j`, `--json` : Output results in JSON format.

## Examples

Calculate subnet details for CIDR notation:

```bash
ipcacl 192.168.1.10/24
```

Calculate subnet details using explicit netmask:

```bash
ipcacl 10.0.0.5 255.255.255.0
```

Show binary representation:

```bash
ipcacl -b 172.16.10.50/20
```

Output as JSON:

```bash
ipcacl -j 192.168.1.100/26
```

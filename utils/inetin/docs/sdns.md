# sdns

`sdns` is an advanced asynchronous DNS query utility built on c-ares, supporting multiple record types, custom DNS servers, and reverse lookups.

## Usage

```bash
sdns [OPTIONS] SITE
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-e RECORD` : Query a specific DNS record type (repeatable). Supported record types: `A`, `AAAA`, `CNAME`, `MX`, `NS`, `TXT`, `PTR`, `SOA`. (Default when not specified: `A`, `AAAA`, `CNAME`).
- `-f FILE` : Write query output to `FILE`.
- `-s SERVER` : Use a single custom DNS server.
- `-ss SERVERS` : Use a comma-bracket separated list of custom DNS servers (e.g. `["1.1.1.1", "8.8.8.8"]`). Mutually exclusive with `-s`.
- `-x` : Perform a reverse DNS lookup (SITE must be an IPv4 or IPv6 address).

## Examples

Query default records (`A`, `AAAA`, `CNAME`) for a domain:
```bash
sdns example.com
```

Query specific record types (`MX` and `TXT`) using a custom DNS server:
```bash
sdns -e MX -e TXT -s 1.1.1.1 example.com
```

Use multiple custom DNS servers and write output to a file:
```bash
sdns -ss '["1.1.1.1", "8.8.8.8"]' -f dns_results.txt example.com
```

Perform a reverse DNS lookup on an IP address:
```bash
sdns -x 93.184.216.34
```

## Exit Status

- `0`: Success.
- Non-zero: Argument error, query failure, or missing parameters.

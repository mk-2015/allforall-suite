# resolv

`resolv` is a lightweight DNS resolution utility built on Boost.Asio, designed for forward and reverse DNS lookups.

## Usage

```bash
resolv [OPTIONS] [SITE]
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-4` : Output IPv4 addresses (default).
- `-6` : Output IPv6 addresses.
- `-a` : Output all IPv4 and IPv6 addresses.
- `-r` : Reverse DNS lookup: resolve an IP address to a hostname.

## Examples

Resolve IPv4 addresses for a domain name:

```bash
resolv example.com
```

Resolve both IPv4 and IPv6 addresses for a URL:

```bash
resolv -a https://example.com/path
```

Resolve IPv6 addresses only:

```bash
resolv -6 example.com
```

Perform a reverse DNS lookup on an IP address:

```bash
resolv -r 93.184.216.34
```

## Exit Status

- `0`: Success (at least one address or hostname found).
- `1`: Lookup failed or no results found.
- `2`: Argument parsing error, conflicting options, missing SITE, or invalid SITE.

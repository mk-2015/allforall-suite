# stls

`stls` (Simple TLS) is a lightweight command-line utility built on OpenSSL for inspecting TLS connections, certificates, cipher suites, ALPN protocols, and verification status of a remote server.

## Usage

```bash
stls [OPTIONS] [SITE]
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-v` : Enable verbose output (prints detailed connection progress and diagnostics).
- `-c` : Show certificate details (Subject, Issuer, Valid From, Valid Until).
- `-j` : Output results in JSON format.
- `--no-verify` : Disable certificate verification.
- `-p PORT` : Specify port (default: `443`).
- `-s HOST` : Specify SNI (Server Name Indication) hostname.
- `-t SECONDS` : Connection timeout in seconds (default: `10`, range: 1 to 3600).

## Examples

- Inspect TLS connection to a domain:

```bash
stls example.com
```

- Show certificate details and use a custom port:

```bash
stls -c -p 8443 example.com
```

- Output connection and certificate info in JSON format with verbose logging:

```bash
stls -j -v -c https://example.com
```

- Specify custom SNI and disable certificate verification:

```bash
stls -s alternative.host --no-verify example.com
```

- Set a custom connection timeout:

```bash
stls -t 5 example.com
```

## Exit Status

- `0`: Success (handshake successful and certificate verified, or verification disabled).
- `1`: Certificate verification failed.
- `2`: Argument parsing error, connection failure, timeout, invalid site, or initialization error.

## Notes

- `stls` supports both domain names and IP literals (IPv4 and IPv6).
- Automatically reports negotiated TLS version, cipher suite, and ALPN (Application-Layer Protocol Negotiation) protocol.

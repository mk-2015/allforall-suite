# furl

`furl` is a versatile command-line URL manipulation utility for parsing, encoding, decoding, extracting components, and constructing URLs.

## Usage

```bash
furl [OPTIONS] [ARGUMENTS...]
```

## Options and Operations

- `-e TEXT` : Percent-encode text.
- `-d TEXT` : Percent-decode text.
- `-qp URL` : Extract all query parameters from a URL.
- `-qp NAME URL` : Extract the value(s) of a specific query parameter from a URL.
- `-h URL` : Extract the host from a URL.
- `-p URL` : Extract the port from a URL.
- `-P URL` : Extract the protocol/scheme from a URL.
- `-q URL` : Extract the path from a URL.
- `-C PROTOCOL HOST PATH [NAME=VALUE ...] [-Cp PORT]` : Construct a URL from components, optional query parameters, and port.
- `--help` : Show help message and exit.

## Examples

Percent-encode a string:

```bash
furl -e "hello world?foo=bar"
```

Percent-decode a string:

```bash
furl -d "hello%20world%3Ffoo%3Dbar"
```

Extract host from a URL:

```bash
furl -h https://example.com:8080/path?query=1
```

Extract protocol:

```bash
furl -P https://example.com
```

Extract port:

```bash
furl -p https://example.com:8443/
```

Extract path:

```bash
furl -q https://example.com/api/v1/users
```

Extract all query parameters:

```bash
furl -qp "https://example.com/?foo=1&bar=2"
```

Extract a specific query parameter value:

```bash
furl -qp foo "https://example.com/?foo=1&bar=2"
```

Construct a URL:

```bash
furl -C https example.com /search q=test page=1 -Cp 443
```

## Exit Status

- `0`: Success.
- `2`: Argument error, parsing failure, or invalid syntax.

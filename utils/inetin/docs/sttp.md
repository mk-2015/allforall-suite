# sttp

`sttp` (Simple HTTP) is a lightweight, robust command-line HTTP client utility built on libcurl, designed for querying websites, testing APIs, downloading files, and performing network transfers.

## Usage

```bash
sttp [OPTIONS] [SITE]
```

## Options

- `-m METHOD`
  HTTP method to use (default: `GET`). Supported methods include `GET`, `HEAD`, `POST`, `PUT`, `DELETE`, `OPTIONS`, `TRACE`, and `PATCH`.
- `-b BODY`
  Request body string to send with the request. (Cannot be used with `-U`).
- `-h NAME VALUE`
  Add a custom request header. This option is repeatable. If `VALUE` is omitted or the next argument starts with `-`, it defaults to `"not-specified"`.
- `-u USERAGENT`
  Override the default User-Agent string.
- `-d FILE`
  Download response body to an existing `FILE`; fails if `FILE` does not exist. (Mutually exclusive with `-D` and `-Dd`).
- `-D FILE`
  Download response body to `FILE`, creating it if it does not exist. (Mutually exclusive with `-d` and `-Dd`).
- `-Dd`
  Download response body using the remote filename extracted from the URL path or the `Content-Disposition` header. (Mutually exclusive with `-d` and `-D`).
- `-R`
  Follow HTTP/HTTPS redirects.
- `-rH`
  Display response headers on `stderr`.
- `-rHd FILE`
  Save response headers to `FILE`.
- `-v`
  Enable verbose diagnostics and print a detailed transfer summary to `stderr`.
- `-U FILE`
  Upload `FILE` as the request body. (Cannot be used with `-b`).
- `-t TIME`
  Total request timeout in milliseconds.
- `-T TIME`
  Connection timeout in milliseconds.
- `-mR NUM`
  Maximum number of redirects (default: unlimited when `-R` is active).
- `-4`
  Force IPv4 address resolution.
- `-6`
  Force IPv6 address resolution.
- `--help`, `-?`
  Show the help menu and usage information.

## Examples

- Basic GET request:
  
```bash
sttp https://example.com
```

- Send a POST request with a custom JSON body and header:

```bash
sttp -m POST -b '{"status":"active"}' -h Content-Type application/json https://api.example.com/v1/status
```

- Download a file using the remote filename:

```bash
sttp -Dd https://example.com/downloads/archive.zip
```

- Follow redirects and display response headers:

```bash
sttp -R -rH https://example.com
```

- Upload a file using PUT:

```bash
sttp -m PUT -U payload.bin https://api.example.com/upload
```

- Verbose request with custom timeout:
  
```bash
sttp -v -t 5000 https://api.example.com/health
```

## Exit Status

- `0`: Success.
- `1`: Argument parsing error or missing required arguments/help requested.
- `2`: Initialization failure, file I/O error, or timeout.
- `3`: libcurl transfer error (network/protocol failure).

## Notes

- `sttp` automatically generates a default User-Agent string including OS information and libcurl version if `-u` is not provided.
- When `-Dd` is used, filename sanitization is applied to protect against malicious or invalid path components in remote filenames.

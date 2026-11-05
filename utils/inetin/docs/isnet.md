# isnet

`isnet` is a network connectivity testing utility that asynchronously probes multiple well-known public websites (via HTTPS/port 443) using Boost.Asio to determine whether active internet connectivity is present.

## Usage

```bash
isnet
```

## Description

`isnet` sends concurrent asynchronous TCP connection probes to a built-in list of prominent public domains and IP addresses. If at least one connection succeeds within the timeout period, it reports success and exits with status `0`. If all connection attempts fail or time out, it reports failure and exits with status `1`.

## Examples

Check internet connectivity:

```bash
isnet
```

## Exit Status

- `0`: Network connectivity detected (at least one successful connection).
- `1`: Network connectivity failure (all connection attempts failed or timed out).

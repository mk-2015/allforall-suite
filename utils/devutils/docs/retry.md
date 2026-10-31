# retry

`retry` runs a command again when it fails, up to a configurable number of attempts.

## Usage

```bash
retry [OPTIONS] <command> [ARGS...]
```

### Options

- `-h` : Display help
- `-n NUM` : Maximum total attempts
- `-d SECONDS` : Delay between attempts
- `-m SECONDS` : Maximum total runtime
- `-q` : Suppress retry messages
- `-v` : Verbose attempt logging

### Examples

# Retry up to 3 attempts
```bash
retry -n 3 curl https://example.com
```

# Wait 2 seconds between attempts
```bash
retry -n 5 -d 2 ./build.sh
```

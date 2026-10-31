# nothing

`nothing` is a utility that performs no operation (or optionally sleeps for a specified duration in milliseconds) and exits with a configurable integer exit code.

## Usage

```bash
nothing [OPTIONS]
```

## Options

- `-s`, `--sleep <MS>` : Sleep in milliseconds.
- `-e`, `--exit-code <CODE>` : Exit with specified integer code (default: 0).
- `-h`, `--help` : Display help message and exit.

## Examples

Sleep for 500 milliseconds and exit with code 0:
```bash
nothing --sleep 500
```

Exit with a custom error code:
```bash
nothing --exit-code 42
```

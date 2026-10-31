# lockout

`lockout` initiates a system session termination (logout or shutdown) after an optional precise millisecond delay.

## Usage

```bash
lockout [OPTIONS]
```

## Options

- `-s`, `--sleep <MS>` : Sleep for exact millisecond duration before lockout.
- `-shut`, `--shutdown` : Trigger system shutdown instead of session logout/termination.
- `-h`, `--help` : Display help message and exit.

## Examples

Lock out or terminate session after 5 seconds:
```bash
lockout --sleep 5000
```

Shutdown the system:
```bash
lockout --shutdown
```

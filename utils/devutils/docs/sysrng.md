# sysrng

`sysrng` is a cryptographic pseudo-random number generator (CSPRNG) utility for generating secure random bytes on supported platforms (Linux, BSD, Windows).

## Synopsis

```bash
sysrng [OPTIONS] [BYTES]
```

## Options

- `-h` : output in hex, can be used with `-f`
- `-b` : output in binary, can be used with `-f`
- `-o` : output in octal, can be used with `-f`
- `-f FILE` : Output data into FILE
- `--help` : Help menu
- `-r` : Output raw bytes, can only be used with `-f`

## Examples

Generate 32 bytes in hex format (default):
```bash
sysrng 32
```

Generate 16 bytes in binary format:
```bash
sysrng -b 16
```

Output 64 raw bytes into a file:
```bash
sysrng -r -f random.bin 64
```

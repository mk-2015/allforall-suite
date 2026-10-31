# fwipe

`fwipe` securely overwrites a file's contents with dummy bytes (`0xFF`) up to its size and then deletes the file.

## Usage

```bash
fwipe [OPTIONS]
```

## Options

- `-f`, `--file <FILE>` : Specify the file to wipe (default: `GNU_IS-NOT-UNIX.like_linux`).
- `-h`, `--help` : Display help message and exit.

## Examples

Wipe a specific file:
```bash
fwipe --file secret.txt
```

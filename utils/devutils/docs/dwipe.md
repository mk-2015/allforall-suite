# dwipe

`dwipe` recursively wipes the contents of a target directory by overwriting all regular files with zero-byte buffers (`0x00`) and subsequently deleting the directory and its contents.

## Usage

```bash
dwipe [OPTIONS]
```

## Options

- `-d`, `--dir <DIR>` : Specify the target directory to wipe (REQUIRED).
- `-h`, `--help` : Display help message and exit.

## Examples

Wipe a target directory and its contents:
```bash
dwipe --dir /tmp/secret_folder
```

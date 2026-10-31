# bmerge

Reassemble binary chunks that were previously split with bsplit.

## Usage

bmerge [OPTIONS] <split-dir>

## Options

- -o, --output <file>
  Output file path for the restored binary
- -p, --prefix <name>
  The chunk prefix to merge. Default: chunk
- -h, --help
  Show the help menu

## Examples

- bmerge file.bin.split
- bmerge file.bin.split -o restored.bin
- bmerge file.bin.split -p datachunk -o restored.bin

## Output

The tool reads matching chunk files in numeric order and concatenates them into one output file.

## Notes

- It expects chunk files to follow the naming pattern `<prefix>-<index>.bin`.
- This is the companion tool to bsplit and is intended for safe rebuilds of segmented data.

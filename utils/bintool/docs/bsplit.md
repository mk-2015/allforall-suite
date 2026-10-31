# bsplit

Split a binary file into a sequence of chunk files.

## Usage

bsplit [OPTIONS] <file>

## Options

- -s, --size <bytes>
  Chunk size in bytes. Default: 1048576
- -o, --output <dir>
  Directory where chunks are written. Default: <file>.split/
- -p, --prefix <name>
  Prefix used for generated chunk filenames. Default: chunk
- -z, --pad <digits>
  Zero-pad the chunk index to the specified width.
- -h, --help
  Show the help menu

## Examples

- bsplit file.bin
- bsplit -s 4096 file.bin
- bsplit -o parts -p datachunk -z 4 file.bin

## Output layout

The default output directory is:

file.bin.split/
    chunk-1.bin
    chunk-2.bin
    chunk-3.bin

With zero-padding enabled:

file.bin.split/
    chunk-0001.bin
    chunk-0002.bin
    chunk-0003.bin

## Notes

- The tool writes each chunk sequentially and preserves byte order.
- This is useful for large binary transfers, archival workflows, or chunked analysis.

# parg

`parg` builds and executes commands from input data (similar to `xargs`), supporting parallel execution, batching by argument or line count, custom delimiters, placeholders, and error handling.

## Usage

```bash
parg [OPTIONS] [--] command [ARGS...]
```

## Options

- `-in FILE` : Read input from `FILE` instead of standard input.
- `-0` : Use NUL (`\0`) as the item delimiter instead of whitespace or newlines.
- `-mx NUM` : Maximum number of arguments (items) per command execution.
- `-L NUM` : Maximum number of lines per command execution.
- `-T NUM` : Number of parallel execution threads (default is 1).
- `-nrun` : Do not run the command if the input items are empty.
- `-d DELIM` : Use a custom string delimiter `DELIM` between input items.
- `-P` : Prompt before executing each command.
- `-p PLACEHOLDER` : Replace `PLACEHOLDER` with each input item in the command.
- `-J PLACEHOLDER` : Replace `PLACEHOLDER` with a batch of input items in the command.
- `-e` : Stop on error (abort execution if any command returns a non-zero exit code).
- `--` : Stop processing options; all subsequent arguments are treated as the command and its arguments.

## Examples

# Run a download command for each URL in parallel with 4 threads
```bash
cat urls.txt | parg -T 4 curl -O
```

# Use a custom placeholder `{}` to copy files
```bash
cat files.txt | parg -p {} cp {} /backup/
```

# Batch input items with a maximum of 5 arguments per execution
```bash
cat items.txt | parg -mx 5 process_batch
```

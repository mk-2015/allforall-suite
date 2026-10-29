# run

`run` executes an application with flexible runtime options, including environment variable overrides, output redirection, working directory configuration, process information reporting, and asynchronous execution.

## Usage

```bash
run [OPTIONS] app [ARGS...]
```

## Options

- `-env:NAME=VALUE` : Set an environment variable for the child process. Can be specified multiple times.
- `-stdout:FILE` : Redirect standard output of the child process to `FILE`.
- `-stderr:FILE` : Redirect standard error of the child process to `FILE`.
- `-stdort:FILE` : Redirect both standard output and standard error of the child process to `FILE`.
- `-dir:DIR` : Change the working directory of the child process to `DIR` before execution.
- `-q` : Suppress `run`'s own error messages and messages.
- `-i` : Print process information (PID, working directory, environment overrides, and redirection targets) to stdout.
- `-Q` : Return immediately after launching the child process (asynchronous execution without waiting for completion).
- `--` : Stop processing options; all subsequent arguments are treated as the application and its arguments.
- `-h`, `--help` : Display help message and exit.

## Examples

Run a command with a custom environment variable and working directory:
```bash
run -dir:/tmp -env:FOO=bar printenv FOO
```

Redirect stdout and stderr to a log file:
```bash
run -stdort:output.log ./my_script.sh
```

Launch a background process asynchronously and print process information:
```bash
run -i -Q ./long_running_server.sh
```

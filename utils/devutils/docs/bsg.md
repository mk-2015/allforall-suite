# bsg

`bsg` (Basic System GUI) is a cross-platform system graphical message notification utility that displays popup dialog boxes (using Win32 on Windows and GTK on Linux) with customizable message severity levels, button layouts, and background command execution.

## Usage

```bash
bsg [OPTIONS] [MESSAGE]
```

## Options

- `-t TYPE` : Specify message type / severity level (`DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`, `CRITICAL`). Default: `INFO`.
- `-T TYPE` : Specify button configuration (`OK`, `OKRETRY`, `OKCANCEL`, `OKRETRYCANCEL`). Default: `OK`.
- `-C CMD` : Run the specified command in the background when the dialog is interacted with/launched.
- `-h`, `--help` : Display help message and exit.
- `-v`, `--version` : Display version information and exit.

## Examples

Display an informational message dialog:

```bash
bsg "Operation completed successfully."
```

Display a warning dialog with an OK/Cancel button layout:

```bash
bsg -t WARN -T OKCANCEL "Are you sure you want to continue?"
```

Display an error dialog and execute a command in the background:

```bash
bsg -t ERROR -C "logger 'Critical system error encountered'" "An unexpected error occurred."
```

## Exit Status

- `0`: Success.
- Non-zero: Argument parsing error, missing message/command, or GUI initialization failure.

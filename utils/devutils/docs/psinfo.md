# psinfo

`psinfo` inspects process details by PID, providing comprehensive information including Process ID, Parent Process ID, process name, full executable path, command line arguments, and resident set size (RSS) memory usage.

## Usage

```bash
psinfo [PID]
```

### Parameters

- `PID` : The Process ID of the target process to inspect.

## Output Details

- **PID** : Target Process ID.
- **PPID** : Parent Process ID.
- **Name** : Process executable filename.
- **Executable** : Full absolute path to the process executable (if accessible).
- **Command Line** : Full command line invocation string (if available).
- **Memory RSS** : Resident Set Size memory consumption in kilobytes (KB).

## Examples

# Inspect process information for PID 1234

```bash
psinfo 1234
```

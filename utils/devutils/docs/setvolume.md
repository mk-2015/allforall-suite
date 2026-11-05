# setvolume

`setvolume` is a cross-platform command-line audio volume control utility (supporting Windows via Core Audio APIs and Unix/Linux) that allows querying and modifying system audio volume levels and mute states.

## Usage

```bash
setvolume [COMMAND]
```

## Commands / Arguments

- *(no arguments)* : Display current volume percentage and mute status.
- `<0-100>` : Set absolute master volume percentage (clamped between `0` and `100`).
- `+<N>` : Increase master volume by `N` percent.
- `-<N>` : Decrease master volume by `N` percent.
- `mute` : Mute audio output.
- `unmute` : Unmute audio output.

## Examples

Show current volume and mute state:

```bash
setvolume
```

Set volume to 75%:

```bash
setvolume 75
```

Increase volume by 10%:

```bash
setvolume +10
```

Decrease volume by 5%:

```bash
setvolume -5
```

Mute audio:

```bash
setvolume mute
```

Unmute audio:

```bash
setvolume unmute
```

## Exit Status

- `0`: Success.
- Non-zero: Invalid arguments, volume range error, or audio backend access failure.

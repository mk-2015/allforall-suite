# var

`var` is a utility for outputting high-precision mathematical constants.

## Usage

```bash
var [CONSTANT] [OPTIONS]
```

## Constants

- `E` : Euler's number ($e$)
- `PI` : Pi ($\pi$)
- `PI2` : Pi squared ($\pi^2$)
- `E2` : Euler's number squared ($e^2$)
- `SQRT2`, `PYTHAGORAS` : Square root of 2 ($\sqrt{2}$)
- `GR`, `GOLDEN_RATIO` : Golden ratio ($\phi$)
- `EM`, `EULER_MASCHERONI` : Euler-Mascheroni constant ($\gamma$)
- `AP`, `APERY` : Apery's constant ($\zeta(3)$)

## Options

- `-h` : Display help message and exit
- `-d` : Output integer part only
- `-f` : Output fractional digits only
- `-p N` : Set precision in decimal places (default: 5, maximum: 100)

## Examples

Print Pi with default precision (5 decimal places):
```bash
var PI
```

Print Euler's number with 20 decimal places of precision:
```bash
var E -p 20
```

Print only the integer part of the Golden Ratio:
```bash
var GOLDEN_RATIO -d
```

Print only the fractional digits of Apery's constant with 10 decimal places:
```bash
var APERY -p 10 -f
```

# Vector2::Scale

```cpp
err Scale(double scalar);
```

Scales current vector components by a double scalar value (`this.x = this.x * scalar`, `this.y = this.y * scalar`).

### Parameters
- `scalar`: Double multiplier.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_OVERFLOW` if the result overflows `INT64` boundaries or if the scalar is `NaN`/`Infinity`.

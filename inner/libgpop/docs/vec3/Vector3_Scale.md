# Vector3::Scale

```cpp
err Scale(double scalar);
```

Scales current vector components by a double scalar value (`this.x *= scalar`, `this.y *= scalar`, `this.z *= scalar`).

### Parameters
- `scalar`: Double multiplier.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_OVERFLOW` if the result overflows `INT64` boundaries.

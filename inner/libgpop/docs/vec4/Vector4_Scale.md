# Vector4::Scale

```cpp
err Scale(double scalar);
```

Scales current vector components by a double scalar value (`this.x *= scalar`, `this.y *= scalar`, `this.z *= scalar`, `this.w *= scalar`).

### Parameters
- `scalar`: Double multiplier.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_OVERFLOW` if the result overflows `INT64` boundaries.

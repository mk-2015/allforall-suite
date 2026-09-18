# Vector2_Scale

```c
err Vector2_Scale(Vector2* a, double scalar);
```

Scales vector `a` components by a double scalar value (`a.x = a.x * scalar`, `a.y = a.y * scalar`).

### Parameters
- `a`: Pointer to the destination `Vector2`.
- `scalar`: Double multiplier.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_NULLVEC` if the vector is `NULL`.
- `EV2_OVERFLOW` if the result overflows `INT64` boundaries or if the scalar is `NaN`/`Infinity`.

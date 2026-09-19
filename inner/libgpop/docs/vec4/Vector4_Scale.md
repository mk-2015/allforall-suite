# Vector4_Scale

```c
err Vector4_Scale(Vector4* a, double scalar);
```

Scales vector `a` components by a double scalar value (`a = a * scalar`).

### Parameters
- `a`: Pointer to the destination `Vector4`.
- `scalar`: Double multiplier.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if `a` is `NULL`.
- `EV4_OVERFLOW` if the result overflows `INT64` boundaries or if the scalar is `NaN`/`Infinity`.

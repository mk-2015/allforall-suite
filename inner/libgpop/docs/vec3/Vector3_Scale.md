# Vector3_Scale

```c
err Vector3_Scale(Vector3* a, double scalar);
```

Scales vector `a` components by a double scalar value (`a.x = a.x * scalar`, `a.y = a.y * scalar`, `a.z = a.z * scalar`).

### Parameters
- `a`: Pointer to the destination `Vector3`.
- `scalar`: Double multiplier.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if the vector is `NULL`.
- `EV3_OVERFLOW` if the result overflows `INT64` boundaries or if the scalar is `NaN`/`Infinity`.

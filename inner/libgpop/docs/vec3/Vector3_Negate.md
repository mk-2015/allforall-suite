# Vector3_Negate

```c
err Vector3_Negate(Vector3* a);
```

Negates components of the vector `a` (`a.x = -a.x`, `a.y = -a.y`, `a.z = -a.z`).

### Parameters
- `a`: Pointer to the target `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if the vector is `NULL`.
- `EV3_UNDERFLOW` if any component contains `INT64_MIN` (which cannot be negated).

# Vector4_Negate

```c
err Vector4_Negate(Vector4* a);
```

Negates components of the vector `a` (`a = -a`).

### Parameters
- `a`: Pointer to the target `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if `a` is `NULL`.
- `EV4_UNDERFLOW` if any component contains `INT64_MIN` (cannot be negated).

# Vector2_Negate

```c
err Vector2_Negate(Vector2* a);
```

Negates both components of the vector `a` (`a.x = -a.x`, `a.y = -a.y`).

### Parameters
- `a`: Pointer to the target `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_NULLVEC` if the vector is `NULL`.
- `EV2_UNDERFLOW` if any component contains `INT64_MIN` (which cannot be negated).

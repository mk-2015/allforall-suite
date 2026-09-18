# Vector2_Set

```c
err Vector2_Set(Vector2* Vector, int64_t x, int64_t y, bool xs, bool ys);
```

Sets coordinates of an existing vector selectively.

### Parameters
- `Vector`: Pointer to the target `Vector2`.
- `x`: The new X value.
- `y`: The new Y value.
- `xs`: If `true`, retains the current X value of the vector, ignoring the `x` parameter.
- `ys`: If `true`, retains the current Y value of the vector, ignoring the `y` parameter.

### Returns
`EV2_SUCCESS` if successful, or `EV2_NULLVEC` if the input vector is `NULL`.

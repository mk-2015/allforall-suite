# Vector4_Set

```c
err Vector4_Set(Vector4* Vector, int64_t x, int64_t y, int64_t z, int64_t w, bool xs, bool ys, bool zs, bool ws);
```

Sets coordinates of an existing vector selectively.

### Parameters
- `Vector`: Pointer to the target `Vector4`.
- `x`, `y`, `z`, `w`: The new coordinate values.
- `xs`, `ys`, `zs`, `ws`: If `true`, retains the current value for that coordinate, ignoring the corresponding input parameter.

### Returns
`EV4_SUCCESS` on success, or `EV4_NULLVEC` if the input vector is `NULL`.

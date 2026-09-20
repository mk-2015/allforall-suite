# Vector4::Cross4D

```cpp
err Cross4D(const Vector4& b, const Vector4& c, Vector4& out) const;
```

Calculates the 4D cross product (pseudo-cross product) of the current vector and vectors `b` and `c`, storing result in `out`.

### Parameters
- `b`: Const reference to second `Vector4`.
- `c`: Const reference to third `Vector4`.
- `out`: Destination `Vector4`.

### Returns
- `EV4_SUCCESS` on success.

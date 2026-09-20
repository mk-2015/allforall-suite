# Vector2::Dot

```cpp
double Dot(const Vector2& b, err* Err) const;
```

Calculates the dot product of the current vector and vector `b` (`this.x * b.x + this.y * b.y`).

### Parameters
- `b`: Const reference to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or `EV2_UB` if calculations hit overflow or undefined boundaries.

### Returns
The dot product as a `double`.

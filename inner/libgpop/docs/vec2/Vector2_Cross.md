# Vector2::Cross

```cpp
double Cross(const Vector2& b, err* Err) const;
```

Calculates the 2D cross product of the current vector and vector `b` (`this.x * b.y - this.y * b.x`).

### Parameters
- `b`: Const reference to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or `EV2_UB` on overflow.

### Returns
The cross product value as a `double`.

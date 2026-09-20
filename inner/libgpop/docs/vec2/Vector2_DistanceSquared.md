# Vector2::DistanceSquared

```cpp
double DistanceSquared(const Vector2& b, err* Err) const;
```

Calculates the squared Euclidean distance between the current vector and vector `b`. This is more performant than `Vector2::Distance` as it avoids a square root calculation.

### Parameters
- `b`: Const reference to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or `EV2_UB` if calculation steps overflow or hit undefined boundaries.

### Returns
The squared distance between the two vectors as a `double`.

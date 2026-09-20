# Vector2::Distance

```cpp
double Distance(const Vector2& b, err* Err) const;
```

Calculates the Euclidean distance between the current vector and vector `b`.

### Parameters
- `b`: Const reference to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or `EV2_UB` if calculation steps overflow or hit undefined boundaries.

### Returns
The distance between the two vectors as a `double`.

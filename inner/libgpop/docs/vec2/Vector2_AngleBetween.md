# Vector2::AngleBetween

```cpp
double AngleBetween(const Vector2& b, err* Err) const;
```

Calculates the angle (in radians) between the current vector and vector `b`.

### Parameters
- `b`: Const reference to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or an error code if the calculation fails (e.g., zero-length vector).

### Returns
The angle in radians as a `double`.

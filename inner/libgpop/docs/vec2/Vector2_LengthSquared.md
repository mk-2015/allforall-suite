# Vector2::LengthSquared

```cpp
double LengthSquared(err* Err) const;
```

Calculates the squared geometric length (magnitude squared) of the current vector. This function is more performant than `Vector2::Length` as it avoids a square root calculation.

### Parameters
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or `EV2_OVERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The squared length of the vector as a `double`.

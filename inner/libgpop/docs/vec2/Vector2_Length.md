# Vector2::Length

```cpp
double Length(err* Err) const;
```

Calculates the geometric length (magnitude) of the current vector.

### Parameters
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or `EV2_UNDERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The length of the vector as a `double`.

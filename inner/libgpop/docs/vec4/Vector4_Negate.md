# Vector4::Negate

```cpp
err Negate();
```

Negates all components of the current vector.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_UNDERFLOW` if any component is `INT64_MIN` (which cannot be negated).

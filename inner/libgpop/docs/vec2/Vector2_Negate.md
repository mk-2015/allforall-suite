# Vector2::Negate

```cpp
err Negate();
```

Negates both components of the current vector (`this.x = -this.x`, `this.y = -this.y`).

### Returns
- `EV2_SUCCESS` on success.
- `EV2_UNDERFLOW` if any component contains `INT64_MIN` (which cannot be negated).

# KdfParams

```cpp
struct KdfParams;
```

Configuration parameters for various KDF algorithms.

### Fields
- `iterations`: Common iteration count.
- `argon2_time`, `argon2_memory`, `argon2_parallel`: Argon2 specific tuning.
- `scrypt_N`, `scrypt_r`, `scrypt_p`: Scrypt specific tuning.
- `bcrypt_cost`: Bcrypt specific tuning.
- `info`, `info_len`: HKDF specific context info.

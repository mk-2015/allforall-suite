# open_random

```c
randalg_t* open_random(ALGORITHM algorithm, RANDOM_SEED seed, MIX isMixOn, MIX_SEED mix_seed);
```

Initializes a new random number generator.

### Parameters
- `algorithm`: The algorithm to use. See `randalg.h` for available `RANDALG_*` constants. Using an unsupported value may result in unpredictable behavior or function failure.
- `seed`: The initial seed for the algorithm. A constant seed will produce a deterministic sequence of random numbers for most algorithms.
- `isMixOn`: Whether mixing is enabled.
    - `MIX_ON` (1): Enables post-generation mixing.
    - `MIX_NO` (0): Disables post-generation mixing.
- `mix_seed`: The seed for the mixing function. This value is used if `isMixOn` is set to `MIX_ON`.

### Returns
Returns a pointer to a `randalg_t` structure, or `NULL` on failure (e.g., memory allocation error). Call `lasterr()` to determine the cause of failure.

# setopt

```c
void setopt(ALGORITHM algorithm, MIX isMixOn, MIX_SEED mix_seed, RANDOM_SEED seed, randalg_t *randalg);
```

Reconfigures an existing random number generator instance with new settings.

### Parameters
- `algorithm`: The new algorithm to use.
- `isMixOn`: Whether mixing is enabled (`MIX_ON` or `MIX_NO`).
- `mix_seed`: The new seed for the mixing function.
- `seed`: The new seed for the algorithm.
- `randalg`: Pointer to the `randalg_t` structure to be reconfigured.

### Usage
This function allows changing the algorithm and seeding configuration of an initialized generator without having to close and reopen it. This is required after calling `reset()`.

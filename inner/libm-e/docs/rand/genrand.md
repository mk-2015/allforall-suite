# genrand

```c
void genrand(randalg_t *randalg);
```

Generates the next random number state for the given generator.

### Parameters
- `randalg`: Pointer to the initialized `randalg_t` structure. If `NULL` or invalid, the operation may fail (check `lasterr()`).

### Behavior
This function updates the internal state of the generator. It **must** be called before `getrand()` to ensure the retrieved random value corresponds to the next number in the generated sequence.

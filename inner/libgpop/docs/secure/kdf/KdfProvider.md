# KdfProvider

```cpp
class KdfProvider;
```

Interface for secure key derivation.

### Methods
- `KdfProvider(gpop::Secure::AlgoNGC Algo)`: Constructor.
- `void SetNeeds(bool nsalt, bool npepper)`: Configures security requirements.
- `void SetParam(KdfParams params)`: Sets KDF-specific tuning parameters.
- `void SetData(...)`: Sets input data (password, salt, pepper).
- `void GenPassword()`: Generates the derived key.
- `gpop::Secure::HashT GetHash() const`: Returns the result.

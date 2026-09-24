# HashProvider

```cpp
class HashProvider;
```

Interface for cryptographic hashing.

### Methods
- `HashProvider(gpop::Secure::AlgoNGC Algo)`: Constructor.
- `void SetNeeds(bool nsalt, bool npepper)`: Configures security requirements.
- `void SetData(const gpop::Secure::Data& indata, const gpop::Secure::Pepper& pepper, const gpop::Secure::Salt& salt)`: Sets input data.
- `void GenHash()`: Generates the hash.
- `gpop::Secure::HashT GetHash()`: Returns the result.

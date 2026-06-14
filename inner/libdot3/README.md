# libdot3
`libdot3` is a small regex engine written in C++ with a thin C wrapper. It builds an NFA from a regex pattern and can test whether a target string matches that pattern.

## C++ API
Namespace: `dotthree::Regex`

### `class regex`
A regex matcher instance.

```cpp
#include <dot3/dot3.hpp>

int main() {
    dotthree::Regex::regex r("a.*b");
    bool matched = r.Parse("axxxb");
    return matched ? 0 : 1;
}
```

#### Public members
- `explicit regex(std::string RegexString);`
  - Constructs a regex matcher from the pattern string.
- `bool Parse(std::string StringToParse);`
  - Tests whether the provided string matches the regex pattern.
  - Returns `true` for a match, `false` otherwise.

### Types
#### `enum StateType`

Defines NFA state kinds used internally.

- `MATCH_CHAR` — match a specific character
- `MATCH_ANY` — match any character
- `MATCH_CLASS` — match a character class
- `SPLIT` — fork control flow in the NFA
- `MATCH_END` — accept state

#### `struct State`
Represents a single state in the NFA.

- `StateType type`
- `char literal`
- `std::set<char> charClass`
- `std::shared_ptr<State> out`
- `std::shared_ptr<State> out1`

#### `struct Fragment`
Represents a partial NFA fragment during construction.

- `std::shared_ptr<State> start`
- `std::vector<std::shared_ptr<State>*> outArrows`

## `str3` / `dotthree::Strings`
`str3` provides a tiny string wrapper type for safer string handling and conversion helpers.

### `class BetterString`

```cpp
#include <dot3/str3.hpp>

int main() {
    dotthree::Strings::BetterString s("123");
    unsigned int value = s.touint();
    std::cout << s << std::endl;
    return 0;
}
```

#### Public members

- `explicit BetterString(std::string InputString);`
  - Constructs a `BetterString` from a standard string.

- `std::string ins();`
  - Returns the underlying `std::string`.

- `char& operator[](size_t index);`
- `const char& operator[](size_t index) const;`
  - Access characters by index.

- `bool operator==(const BetterString& other) const;`
  - Compare two `BetterString` objects for equality.

- `BetterString& operator+=(const BetterString& other);`
- `BetterString operator+(const BetterString& other) const;`
  - Append/concatenate `BetterString` values.

- `BetterString& operator=(const BetterString& other);`
  - Copy assignment.

- `int toint();`
  - Parses the string as an `int`, returning `0` on parse failure.

- `unsigned int touint();`
  - Parses the string as an `unsigned int`, returning `0` on parse failure.

### Helper functions

- `dotthree::Strings::BetterString cut(std::string s);`
  - Constructs a `BetterString` from `s`.

- `std::string uncut(dotthree::Strings::BetterString s);`
  - Returns the underlying `std::string` from a `BetterString`.

## C API
The C wrapper is declared in `include/dot3/dot3.h`.

```c
#include <dot3/dot3.h>

int main(void) {
    void* handle = NewRegexHandler("a.*b");
    bool matched = ParseRegexString(handle, "axxxb");
    CloseRegexHandler(handle);
    return matched ? 0 : 1;
}
```

### C wrapper functions

- `void* NewRegexHandler(const char* regexString);`
  - Create a regex handler for the given pattern.
- `bool ParseRegexString(void* handle, const char* StringToParse);`
  - Returns `true` if the string matches the pattern.
- `void CloseRegexHandler(void* handle);`
  - Frees the handler.

## Build

From the `inner/libdot3` directory, run:

```bash
make all
```

This produces both the static library and shared library outputs.

## Notes

- The implementation is a classic NFA-based regex engine.
- Only the public `regex` class and the C wrapper are intended for use by consumers.

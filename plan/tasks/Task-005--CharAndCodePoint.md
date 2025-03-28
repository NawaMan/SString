# Task-005: Character and Code Point Support

## Motivation
SString needs Java-like character and code point access methods. This requires a Char class to handle UTF-16 code units and methods to work with full Unicode code points.

## Description
1. Create Char class for UTF-16 code unit representation
2. Add character access methods to SString
3. Add code point access methods to SString

## Acceptance Criteria

### Phase 1: Char Class ✓
- [x] Create Char class with:
    - [x] 16-bit storage (char16_t)
    - [x] Constructors for char, char16_t, char32_t
    - [x] Value comparison operators
    - [x] Methods: isHighSurrogate(), isLowSurrogate()
    - [x] Conversion methods: toCodePoint() (with another Char)

### Phase 2: Character Access ✓
- [x] Add to SString:
    - [x] charAt(size_t): Get Char at index
    - [x] operator[]: Same as charAt
    - [x] charValue(size_t): Get raw char16_t value
    - [x] Proper bounds checking
    - [x] UTF-16 caching for performance

### Phase 3: Code Point Access
- [ ] Add to SString:
    - [ ] codePointAt(size_t): Get code point at index
    - [ ] codePointBefore(size_t): Get code point before index
    - [ ] codePointCount(): Count actual code points
    - [ ] offsetByCodePoints(): Navigate by code points

## Implementation Details

1. Char Storage:
   ```cpp
   class Char {
       char16_t value_;  // UTF-16 code unit
   };
   ```

2. Key Features:
   - Zero-overhead abstraction over char16_t
   - Safe surrogate pair handling
   - Seamless integration with SString

3. Error Handling:
   - Invalid index: StringIndexOutOfBoundsException
   - Invalid surrogate pairs: IllegalArgumentException

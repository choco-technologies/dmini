# test_dmini - DMINI Test Application

This is a DMOD application module that comprehensively tests the dmini INI parser module.

## Overview

test_dmini is a DMF (DMOD Module Format) application that loads the dmini module and runs a series of tests to verify all API functions work correctly.

## Usage

### With dmod_loader

```bash
# Load both dmini and test_dmini modules
dmod_loader dmini.dmf test_dmini.dmf
```

### Expected Output

```
=== DMINI Functionality Tests ===

TEST: Create and destroy context
  PASSED
TEST: Parse simple INI string
  PASSED
TEST: Set and get values
  PASSED
TEST: Check section and key existence
  PASSED
TEST: Remove section and key
  PASSED
TEST: Generate INI string
  PASSED
TEST: File read/write
  PASSED
TEST: Handle comments and whitespace
  PASSED

=== Test Summary ===
Passed: 8
Failed: 0
All tests PASSED!
```

## Test Coverage

The application tests the following dmini functionality:

1. **Context Management**
   - `dmini_create()` - Create INI context
   - `dmini_destroy()` - Free INI context

2. **Parsing**
   - `dmini_parse_string()` - Parse from string
   - `dmini_parse_file()` - Parse from file (line-by-line)
   - Comment handling (`;` and `#`)
   - Whitespace trimming

3. **Data Access**
   - `dmini_get_string()` - Get string values with defaults
   - `dmini_get_int()` - Get integer values with defaults
   - `dmini_set_string()` - Set string values
   - `dmini_set_int()` - Set integer values

4. **Queries**
   - `dmini_has_section()` - Check section existence
   - `dmini_has_key()` - Check key existence

5. **Removal**
   - `dmini_remove_section()` - Remove entire section
   - `dmini_remove_key()` - Remove single key

6. **Generation**
   - `dmini_generate_string()` - Generate to buffer (size query supported)
   - `dmini_generate_file()` - Write directly to file

7. **Edge Cases**
   - Global section (keys without section header)
   - Comments (`;` and `#`)
   - Whitespace trimming
   - Empty sections

## Integration with CI

The test application is automatically run as part of the CI pipeline:

```yaml
- name: Run tests with dmod_loader
  run: |
    dmod_loader build/dmf/dmini.dmf build/dmf/test_dmini.dmf
```

## Exit Codes

- `0` - All tests passed
- `1` - One or more tests failed

## Memory Footprint

- RAM: 432 bytes
- ROM: 7 KB

## Building

The test application is built automatically when building the dmini project:

```bash
mkdir build
cd build
cmake .. -DDMOD_MODE=DMOD_MODULE
cmake --build .
```

This generates `dmf/test_dmini.dmf` which can be loaded with dmod_loader.

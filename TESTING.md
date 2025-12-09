# Testing dmini Module

This document describes how to test the dmini INI file parser module.

## Manual Testing

### Prerequisites
- DMOD system built and available
- dmini module built (dmini.dmf file)

### Test 1: Basic Parsing

Create a test INI file `test.ini`:
```ini
; Test INI file
global_key=global_value

[section1]
key1=value1
key2=value2

[section2]
number=42
name=test
```

Load the dmini module and test parsing:
```c
dmini_context_t ctx = dmini_create();
dmini_parse_file(ctx, "test.ini");

// Verify global section
const char* val = dmini_get_string(ctx, NULL, "global_key", "");
assert(strcmp(val, "global_value") == 0);

// Verify section1
val = dmini_get_string(ctx, "section1", "key1", "");
assert(strcmp(val, "value1") == 0);

// Verify section2
int num = dmini_get_int(ctx, "section2", "number", 0);
assert(num == 42);

dmini_destroy(ctx);
```

### Test 2: Setting Values

```c
dmini_context_t ctx = dmini_create();

// Set values
dmini_set_string(ctx, "database", "host", "localhost");
dmini_set_int(ctx, "database", "port", 5432);

// Generate output
dmini_generate_file(ctx, "output.ini");

dmini_destroy(ctx);
```

Expected output.ini:
```ini
[database]
host=localhost
port=5432
```

### Test 3: Line-by-line Parsing

The module reads INI files line by line (max 256 chars per line) to conserve memory on embedded systems.

Test with large file (1000+ lines) to verify memory efficiency.

### Test 4: Generate String with Buffer

```c
dmini_context_t ctx = dmini_create();
dmini_set_string(ctx, "section1", "key1", "value1");

// Get required size
int size = dmini_generate_string(ctx, NULL, 0);

// Allocate buffer
char* buffer = malloc(size);

// Generate
dmini_generate_string(ctx, buffer, size);

printf("%s\n", buffer);
free(buffer);
dmini_destroy(ctx);
```

### Test 5: Edge Cases

Test comment handling:
```ini
; Comment
# Another comment
key=value  ; inline comment not supported
```

Test whitespace trimming:
```ini
  key1  =  value1  
[  section1  ]
  key2 = value2  
```

Test empty sections:
```ini
[empty_section]

[section_with_data]
key=value
```

## API Verification Checklist

- [ ] `dmini_create()` - Returns valid context
- [ ] `dmini_destroy()` - Frees all memory
- [ ] `dmini_parse_string()` - Parses INI from string
- [ ] `dmini_parse_file()` - Reads file line by line
- [ ] `dmini_generate_string()` - Returns size when buffer=NULL, fills buffer when provided
- [ ] `dmini_generate_file()` - Writes directly to file without allocating large buffer
- [ ] `dmini_get_string()` - Returns value or default
- [ ] `dmini_get_int()` - Parses integer correctly
- [ ] `dmini_set_string()` - Creates section/key as needed
- [ ] `dmini_set_int()` - Converts int to string correctly (handles INT_MIN)
- [ ] `dmini_has_section()` - Checks section existence
- [ ] `dmini_has_key()` - Checks key existence
- [ ] `dmini_remove_section()` - Removes section and all keys
- [ ] `dmini_remove_key()` - Removes single key

## Memory Constraints

The module is designed for embedded systems with limited RAM:
- Reads files line by line (256 byte buffer)
- Writes files line by line (256 byte buffer)
- Uses linked lists for dynamic sizing
- No large temporary buffers
- User controls buffer allocation for string generation

## Integration Testing

To test with actual DMOD system:
```bash
# Load module
dmod load dmini.dmf

# In your application
dmini_context_t ctx = dmini_create();
// ... use API
dmini_destroy(ctx);
```

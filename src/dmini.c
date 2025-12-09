#define DMOD_ENABLE_REGISTRATION    ON
#include "dmod.h"
#include "dmini.h"

// String function declarations (provided by DMOD module)
size_t strlen(const char *s);
void *memcpy(void *dest, const void *src, size_t n);
int strcmp(const char *s1, const char *s2);

/**
 * @brief Key-value pair structure
 */
typedef struct dmini_pair
{
    char* key;
    char* value;
    struct dmini_pair* next;
} dmini_pair_t;

/**
 * @brief Section structure
 */
typedef struct dmini_section
{
    char* name;
    dmini_pair_t* pairs;
    struct dmini_section* next;
} dmini_section_t;

/**
 * @brief INI context structure
 */
struct dmini_context
{
    dmini_section_t* sections;
};

// ============================================================================
//                      Helper Functions
// ============================================================================

/**
 * @brief Trim whitespace from beginning and end of string
 * 
 * @param str String to trim (modified in place)
 * @return Pointer to trimmed string
 */
static char* trim_whitespace(char* str)
{
    char* end;
    
    // Trim leading space
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')
    {
        str++;
    }
    
    if (*str == 0)
    {
        return str;
    }
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
    {
        end--;
    }
    
    // Write new null terminator
    *(end + 1) = '\0';
    
    return str;
}

/**
 * @brief Duplicate string using SAL malloc
 */
static char* string_duplicate(const char* str)
{
    if (!str)
    {
        return NULL;
    }
    
    size_t len = strlen(str);
    char* result = (char*)Dmod_Malloc(len + 1);
    if (result)
    {
        memcpy(result, str, len + 1);
    }
    return result;
}

/**
 * @brief Find section by name
 */
static dmini_section_t* find_section(dmini_context_t ctx, const char* section_name)
{
    if (!ctx)
    {
        return NULL;
    }
    
    dmini_section_t* section = ctx->sections;
    while (section)
    {
        if (section->name == NULL && section_name == NULL)
        {
            return section;
        }
        if (section->name && section_name && strcmp(section->name, section_name) == 0)
        {
            return section;
        }
        section = section->next;
    }
    
    return NULL;
}

/**
 * @brief Find key-value pair in section
 */
static dmini_pair_t* find_pair(dmini_section_t* section, const char* key)
{
    if (!section || !key)
    {
        return NULL;
    }
    
    dmini_pair_t* pair = section->pairs;
    while (pair)
    {
        if (strcmp(pair->key, key) == 0)
        {
            return pair;
        }
        pair = pair->next;
    }
    
    return NULL;
}

/**
 * @brief Create new section
 */
static dmini_section_t* create_section(const char* name)
{
    dmini_section_t* section = (dmini_section_t*)Dmod_Malloc(sizeof(dmini_section_t));
    if (!section)
    {
        return NULL;
    }
    
    section->name = name ? string_duplicate(name) : NULL;
    section->pairs = NULL;
    section->next = NULL;
    
    return section;
}

/**
 * @brief Create new key-value pair
 */
static dmini_pair_t* create_pair(const char* key, const char* value)
{
    dmini_pair_t* pair = (dmini_pair_t*)Dmod_Malloc(sizeof(dmini_pair_t));
    if (!pair)
    {
        return NULL;
    }
    
    pair->key = string_duplicate(key);
    pair->value = string_duplicate(value);
    pair->next = NULL;
    
    if (!pair->key || !pair->value)
    {
        if (pair->key) Dmod_Free(pair->key);
        if (pair->value) Dmod_Free(pair->value);
        Dmod_Free(pair);
        return NULL;
    }
    
    return pair;
}

/**
 * @brief Free key-value pair
 */
static void free_pair(dmini_pair_t* pair)
{
    if (!pair)
    {
        return;
    }
    
    if (pair->key)
    {
        Dmod_Free(pair->key);
    }
    if (pair->value)
    {
        Dmod_Free(pair->value);
    }
    Dmod_Free(pair);
}

/**
 * @brief Free section and all its pairs
 */
static void free_section(dmini_section_t* section)
{
    if (!section)
    {
        return;
    }
    
    // Free all pairs
    dmini_pair_t* pair = section->pairs;
    while (pair)
    {
        dmini_pair_t* next = pair->next;
        free_pair(pair);
        pair = next;
    }
    
    // Free section name
    if (section->name)
    {
        Dmod_Free(section->name);
    }
    
    Dmod_Free(section);
}

/**
 * @brief Get or create section
 */
static dmini_section_t* get_or_create_section(dmini_context_t ctx, const char* section_name)
{
    if (!ctx)
    {
        return NULL;
    }
    
    // Try to find existing section
    dmini_section_t* section = find_section(ctx, section_name);
    if (section)
    {
        return section;
    }
    
    // Create new section
    section = create_section(section_name);
    if (!section)
    {
        return NULL;
    }
    
    // Add to list
    if (!ctx->sections)
    {
        ctx->sections = section;
    }
    else
    {
        dmini_section_t* last = ctx->sections;
        while (last->next)
        {
            last = last->next;
        }
        last->next = section;
    }
    
    return section;
}

/**
 * @brief Set key-value pair in section
 */
static int set_pair_in_section(dmini_section_t* section, const char* key, const char* value)
{
    if (!section || !key)
    {
        return DMINI_ERR_INVALID;
    }
    
    // Try to find existing pair
    dmini_pair_t* pair = find_pair(section, key);
    if (pair)
    {
        // Update value
        if (pair->value)
        {
            Dmod_Free(pair->value);
        }
        pair->value = string_duplicate(value);
        if (!pair->value)
        {
            return DMINI_ERR_MEMORY;
        }
        return DMINI_OK;
    }
    
    // Create new pair
    pair = create_pair(key, value);
    if (!pair)
    {
        return DMINI_ERR_MEMORY;
    }
    
    // Add to list
    if (!section->pairs)
    {
        section->pairs = pair;
    }
    else
    {
        dmini_pair_t* last = section->pairs;
        while (last->next)
        {
            last = last->next;
        }
        last->next = pair;
    }
    
    return DMINI_OK;
}

// ============================================================================
//                      Module Interface Implementation
// ============================================================================

/**
 * @brief Module initialization (optional)
 */
void dmod_preinit(void)
{
    // Nothing to do
}

/**
 * @brief Module initialization
 */
int dmod_init(const Dmod_Config_t *Config)
{
    // Nothing to do
    return 0;
}

/**
 * @brief Module deinitialization
 */
void dmod_deinit(void)
{
    // Nothing to do
}

// ============================================================================
//                      Public API Implementation
// ============================================================================

dmini_context_t dmini_create(void)
{
    dmini_context_t ctx = (dmini_context_t)Dmod_Malloc(sizeof(struct dmini_context));
    if (!ctx)
    {
        return NULL;
    }
    
    ctx->sections = NULL;
    
    // Create global section (unnamed section for keys without section)
    ctx->sections = create_section(NULL);
    if (!ctx->sections)
    {
        Dmod_Free(ctx);
        return NULL;
    }
    
    return ctx;
}

void dmini_destroy(dmini_context_t ctx)
{
    if (!ctx)
    {
        return;
    }
    
    // Free all sections
    dmini_section_t* section = ctx->sections;
    while (section)
    {
        dmini_section_t* next = section->next;
        free_section(section);
        section = next;
    }
    
    Dmod_Free(ctx);
}

int dmini_parse_string(dmini_context_t ctx, const char* data)
{
    if (!ctx || !data)
    {
        return DMINI_ERR_INVALID;
    }
    
    // Duplicate data so we can modify it
    char* buffer = string_duplicate(data);
    if (!buffer)
    {
        return DMINI_ERR_MEMORY;
    }
    
    dmini_section_t* current_section = ctx->sections; // Start with global section
    
    char* line = buffer;
    char* next_line;
    
    while (line && *line)
    {
        // Find end of line
        next_line = line;
        while (*next_line && *next_line != '\n' && *next_line != '\r')
        {
            next_line++;
        }
        
        // Null terminate current line
        if (*next_line)
        {
            *next_line = '\0';
            next_line++;
            // Skip \r\n combinations
            if (*next_line == '\n' || *next_line == '\r')
            {
                next_line++;
            }
        }
        
        // Trim whitespace
        line = trim_whitespace(line);
        
        // Skip empty lines and comments
        if (*line == '\0' || *line == ';' || *line == '#')
        {
            line = next_line;
            continue;
        }
        
        // Check for section header
        if (*line == '[')
        {
            char* section_end = line + 1;
            while (*section_end && *section_end != ']')
            {
                section_end++;
            }
            
            if (*section_end == ']')
            {
                *section_end = '\0';
                char* section_name = trim_whitespace(line + 1);
                
                current_section = get_or_create_section(ctx, section_name);
                if (!current_section)
                {
                    Dmod_Free(buffer);
                    return DMINI_ERR_MEMORY;
                }
            }
            
            line = next_line;
            continue;
        }
        
        // Parse key=value
        char* equals = line;
        while (*equals && *equals != '=')
        {
            equals++;
        }
        
        if (*equals == '=')
        {
            *equals = '\0';
            char* key = trim_whitespace(line);
            char* value = trim_whitespace(equals + 1);
            
            if (*key)
            {
                int result = set_pair_in_section(current_section, key, value);
                if (result != DMINI_OK)
                {
                    Dmod_Free(buffer);
                    return result;
                }
            }
        }
        
        line = next_line;
    }
    
    Dmod_Free(buffer);
    return DMINI_OK;
}

int dmini_parse_file(dmini_context_t ctx, const char* filename)
{
    if (!ctx || !filename)
    {
        return DMINI_ERR_INVALID;
    }
    
    // Open file using SAL
    void* file = Dmod_FileOpen(filename, "r");
    if (!file)
    {
        return DMINI_ERR_FILE;
    }
    
    // Get file size
    size_t file_size = Dmod_FileSize(file);
    
    // Allocate buffer
    char* buffer = (char*)Dmod_Malloc(file_size + 1);
    if (!buffer)
    {
        Dmod_FileClose(file);
        return DMINI_ERR_MEMORY;
    }
    
    // Read file
    size_t bytes_read = Dmod_FileRead(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    Dmod_FileClose(file);
    
    // Parse string
    int result = dmini_parse_string(ctx, buffer);
    Dmod_Free(buffer);
    
    return result;
}

char* dmini_generate_string(dmini_context_t ctx)
{
    if (!ctx)
    {
        return NULL;
    }
    
    // Calculate required buffer size
    size_t buffer_size = 0;
    
    dmini_section_t* section = ctx->sections;
    while (section)
    {
        // Section header (skip global section)
        if (section->name)
        {
            buffer_size += strlen(section->name) + 3; // [name]\n
        }
        
        // Key-value pairs
        dmini_pair_t* pair = section->pairs;
        while (pair)
        {
            buffer_size += strlen(pair->key) + strlen(pair->value) + 2; // key=value\n
            pair = pair->next;
        }
        
        // Empty line after section
        if (section->name && section->next)
        {
            buffer_size += 1;
        }
        
        section = section->next;
    }
    
    // Allocate buffer
    char* buffer = (char*)Dmod_Malloc(buffer_size + 1);
    if (!buffer)
    {
        return NULL;
    }
    
    // Generate INI string
    char* pos = buffer;
    section = ctx->sections;
    
    while (section)
    {
        // Section header (skip global section)
        if (section->name)
        {
            *pos++ = '[';
            size_t name_len = strlen(section->name);
            memcpy(pos, section->name, name_len);
            pos += name_len;
            *pos++ = ']';
            *pos++ = '\n';
        }
        
        // Key-value pairs
        dmini_pair_t* pair = section->pairs;
        while (pair)
        {
            size_t key_len = strlen(pair->key);
            memcpy(pos, pair->key, key_len);
            pos += key_len;
            *pos++ = '=';
            size_t value_len = strlen(pair->value);
            memcpy(pos, pair->value, value_len);
            pos += value_len;
            *pos++ = '\n';
            pair = pair->next;
        }
        
        // Empty line after section
        if (section->name && section->next)
        {
            *pos++ = '\n';
        }
        
        section = section->next;
    }
    
    *pos = '\0';
    
    return buffer;
}

int dmini_generate_file(dmini_context_t ctx, const char* filename)
{
    if (!ctx || !filename)
    {
        return DMINI_ERR_INVALID;
    }
    
    // Generate string
    char* data = dmini_generate_string(ctx);
    if (!data)
    {
        return DMINI_ERR_MEMORY;
    }
    
    // Open file for writing
    void* file = Dmod_FileOpen(filename, "w");
    if (!file)
    {
        Dmod_Free(data);
        return DMINI_ERR_FILE;
    }
    
    // Write data
    size_t data_len = strlen(data);
    size_t bytes_written = Dmod_FileWrite(data, 1, data_len, file);
    
    Dmod_FileClose(file);
    Dmod_Free(data);
    
    if (bytes_written != data_len)
    {
        return DMINI_ERR_FILE;
    }
    
    return DMINI_OK;
}

const char* dmini_get_string(dmini_context_t ctx, const char* section, const char* key, const char* default_value)
{
    if (!ctx || !key)
    {
        return default_value;
    }
    
    dmini_section_t* sec = find_section(ctx, section);
    if (!sec)
    {
        return default_value;
    }
    
    dmini_pair_t* pair = find_pair(sec, key);
    if (!pair)
    {
        return default_value;
    }
    
    return pair->value;
}

int dmini_get_int(dmini_context_t ctx, const char* section, const char* key, int default_value)
{
    const char* value = dmini_get_string(ctx, section, key, NULL);
    if (!value)
    {
        return default_value;
    }
    
    // Simple integer conversion
    int result = 0;
    int sign = 1;
    const char* p = value;
    
    // Skip whitespace
    while (*p == ' ' || *p == '\t')
    {
        p++;
    }
    
    // Check for sign
    if (*p == '-')
    {
        sign = -1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }
    
    // Convert digits
    while (*p >= '0' && *p <= '9')
    {
        result = result * 10 + (*p - '0');
        p++;
    }
    
    return result * sign;
}

int dmini_set_string(dmini_context_t ctx, const char* section, const char* key, const char* value)
{
    if (!ctx || !key || !value)
    {
        return DMINI_ERR_INVALID;
    }
    
    dmini_section_t* sec = get_or_create_section(ctx, section);
    if (!sec)
    {
        return DMINI_ERR_MEMORY;
    }
    
    return set_pair_in_section(sec, key, value);
}

int dmini_set_int(dmini_context_t ctx, const char* section, const char* key, int value)
{
    // Convert integer to string
    char buffer[32];
    char* p = buffer + sizeof(buffer) - 1;
    *p = '\0';
    
    int is_negative = 0;
    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }
    
    // Convert digits
    do
    {
        *--p = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    // Add sign
    if (is_negative)
    {
        *--p = '-';
    }
    
    return dmini_set_string(ctx, section, key, p);
}

int dmini_has_section(dmini_context_t ctx, const char* section)
{
    if (!ctx)
    {
        return 0;
    }
    
    return find_section(ctx, section) ? 1 : 0;
}

int dmini_has_key(dmini_context_t ctx, const char* section, const char* key)
{
    if (!ctx || !key)
    {
        return 0;
    }
    
    dmini_section_t* sec = find_section(ctx, section);
    if (!sec)
    {
        return 0;
    }
    
    return find_pair(sec, key) ? 1 : 0;
}

int dmini_remove_section(dmini_context_t ctx, const char* section)
{
    if (!ctx || !section)
    {
        return DMINI_ERR_INVALID;
    }
    
    dmini_section_t* prev = NULL;
    dmini_section_t* curr = ctx->sections;
    
    while (curr)
    {
        if (curr->name && strcmp(curr->name, section) == 0)
        {
            // Remove from list
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                ctx->sections = curr->next;
            }
            
            free_section(curr);
            return DMINI_OK;
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    return DMINI_ERR_NOT_FOUND;
}

int dmini_remove_key(dmini_context_t ctx, const char* section, const char* key)
{
    if (!ctx || !key)
    {
        return DMINI_ERR_INVALID;
    }
    
    dmini_section_t* sec = find_section(ctx, section);
    if (!sec)
    {
        return DMINI_ERR_NOT_FOUND;
    }
    
    dmini_pair_t* prev = NULL;
    dmini_pair_t* curr = sec->pairs;
    
    while (curr)
    {
        if (strcmp(curr->key, key) == 0)
        {
            // Remove from list
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                sec->pairs = curr->next;
            }
            
            free_pair(curr);
            return DMINI_OK;
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    return DMINI_ERR_NOT_FOUND;
}

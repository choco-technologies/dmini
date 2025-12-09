#define DMOD_ENABLE_REGISTRATION ON
#include "dmod.h"
#include "dmini.h"
#include <string.h>

/**
 * @brief Test application for dmini module
 * 
 * This application tests the dmini INI parser module functionality.
 * It can be loaded with dmod_loader along with the dmini module.
 */

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

// Helper macros
#define TEST_START(name) DMOD_LOG_INFO("TEST: %s\n", name)
#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        DMOD_LOG_ERROR("  FAILED: %s\n", msg); \
        tests_failed++; \
        return; \
    } \
} while(0)
#define TEST_PASS() do { \
    DMOD_LOG_INFO("  PASSED\n"); \
    tests_passed++; \
} while(0)

/**
 * @brief Test: Create and destroy context
 */
static void test_create_destroy(void)
{
    TEST_START("Create and destroy context");
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Test: Parse simple INI string
 */
static void test_parse_string(void)
{
    TEST_START("Parse simple INI string");
    
    const char* ini_data = 
        "global_key=global_value\n"
        "\n"
        "[section1]\n"
        "key1=value1\n"
        "key2=value2\n"
        "\n"
        "[section2]\n"
        "number=42\n";
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    int result = dmini_parse_string(ctx, ini_data);
    TEST_ASSERT(result == 0, "Failed to parse string");
    
    // Check global section
    const char* val = dmini_get_string(ctx, NULL, "global_key", "");
    TEST_ASSERT(strcmp(val, "global_value") == 0, "Wrong global value");
    
    // Check section1
    val = dmini_get_string(ctx, "section1", "key1", "");
    TEST_ASSERT(strcmp(val, "value1") == 0, "Wrong section1/key1 value");
    
    val = dmini_get_string(ctx, "section1", "key2", "");
    TEST_ASSERT(strcmp(val, "value2") == 0, "Wrong section1/key2 value");
    
    // Check section2
    int num = dmini_get_int(ctx, "section2", "number", 0);
    TEST_ASSERT(num == 42, "Wrong integer value");
    
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Test: Set and get values
 */
static void test_set_get(void)
{
    TEST_START("Set and get values");
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    // Set string
    int result = dmini_set_string(ctx, "database", "host", "localhost");
    TEST_ASSERT(result == 0, "Failed to set string");
    
    const char* val = dmini_get_string(ctx, "database", "host", "");
    TEST_ASSERT(strcmp(val, "localhost") == 0, "Wrong retrieved value");
    
    // Set integer
    result = dmini_set_int(ctx, "database", "port", 5432);
    TEST_ASSERT(result == 0, "Failed to set integer");
    
    int num = dmini_get_int(ctx, "database", "port", 0);
    TEST_ASSERT(num == 5432, "Wrong integer value");
    
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Test: Has section and key
 */
static void test_has_section_key(void)
{
    TEST_START("Check section and key existence");
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    dmini_set_string(ctx, "section1", "key1", "value1");
    
    TEST_ASSERT(dmini_has_section(ctx, "section1") == 1, "Section should exist");
    TEST_ASSERT(dmini_has_section(ctx, "section2") == 0, "Section should not exist");
    
    TEST_ASSERT(dmini_has_key(ctx, "section1", "key1") == 1, "Key should exist");
    TEST_ASSERT(dmini_has_key(ctx, "section1", "key2") == 0, "Key should not exist");
    
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Test: Remove section and key
 */
static void test_remove(void)
{
    TEST_START("Remove section and key");
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    dmini_set_string(ctx, "section1", "key1", "value1");
    dmini_set_string(ctx, "section1", "key2", "value2");
    
    // Remove key
    int result = dmini_remove_key(ctx, "section1", "key1");
    TEST_ASSERT(result == 0, "Failed to remove key");
    TEST_ASSERT(dmini_has_key(ctx, "section1", "key1") == 0, "Key should be removed");
    TEST_ASSERT(dmini_has_key(ctx, "section1", "key2") == 1, "Key2 should still exist");
    
    // Remove section
    result = dmini_remove_section(ctx, "section1");
    TEST_ASSERT(result == 0, "Failed to remove section");
    TEST_ASSERT(dmini_has_section(ctx, "section1") == 0, "Section should be removed");
    
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Simple substring search (replacement for strstr)
 */
static const char* find_substring(const char* haystack, const char* needle)
{
    if (!haystack || !needle) return NULL;
    if (*needle == '\0') return haystack;
    
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return haystack;
        }
    }
    
    return NULL;
}

/**
 * @brief Test: Generate string
 */
static void test_generate_string(void)
{
    TEST_START("Generate INI string");
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    dmini_set_string(ctx, NULL, "global", "value");
    dmini_set_string(ctx, "section1", "key1", "value1");
    
    // Get required size
    int size = dmini_generate_string(ctx, NULL, 0);
    TEST_ASSERT(size > 0, "Failed to get required size");
    
    // Allocate buffer
    char* buffer = (char*)Dmod_Malloc(size);
    TEST_ASSERT(buffer != NULL, "Failed to allocate buffer");
    
    // Generate string
    int result = dmini_generate_string(ctx, buffer, size);
    TEST_ASSERT(result == size, "Wrong size returned");
    
    // Check content
    TEST_ASSERT(find_substring(buffer, "global=value") != NULL, "Missing global key");
    TEST_ASSERT(find_substring(buffer, "[section1]") != NULL, "Missing section header");
    TEST_ASSERT(find_substring(buffer, "key1=value1") != NULL, "Missing section key");
    
    Dmod_Free(buffer);
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Test: File I/O
 */
static void test_file_io(void)
{
    TEST_START("File read/write");
    
    const char* test_file = "/tmp/test_dmini.ini";
    const char* test_data = 
        "[section1]\n"
        "key1=value1\n"
        "\n"
        "[section2]\n"
        "key2=value2\n";
    
    // Write test file
    void* file = Dmod_FileOpen(test_file, "w");
    TEST_ASSERT(file != NULL, "Failed to create test file");
    Dmod_FileWrite(test_data, 1, strlen(test_data), file);
    Dmod_FileClose(file);
    
    // Parse file
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    int result = dmini_parse_file(ctx, test_file);
    TEST_ASSERT(result == 0, "Failed to parse file");
    
    const char* val = dmini_get_string(ctx, "section1", "key1", "");
    TEST_ASSERT(strcmp(val, "value1") == 0, "Wrong value from file");
    
    // Generate to different file
    const char* output_file = "/tmp/test_dmini_output.ini";
    result = dmini_generate_file(ctx, output_file);
    TEST_ASSERT(result == 0, "Failed to generate file");
    
    // Verify output file exists
    file = Dmod_FileOpen(output_file, "r");
    TEST_ASSERT(file != NULL, "Output file not created");
    Dmod_FileClose(file);
    
    // Clean up
    Dmod_FileRemove(test_file);
    Dmod_FileRemove(output_file);
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Test: Comments and whitespace
 */
static void test_comments_whitespace(void)
{
    TEST_START("Handle comments and whitespace");
    
    const char* ini_data = 
        "; Comment line\n"
        "  key1  =  value1  \n"
        "# Another comment\n"
        "\n"
        "[  section1  ]\n"
        "  key2 = value2  \n";
    
    dmini_context_t ctx = dmini_create();
    TEST_ASSERT(ctx != NULL, "Failed to create context");
    
    int result = dmini_parse_string(ctx, ini_data);
    TEST_ASSERT(result == 0, "Failed to parse string with comments");
    
    const char* val = dmini_get_string(ctx, NULL, "key1", "");
    TEST_ASSERT(strcmp(val, "value1") == 0, "Whitespace not trimmed");
    
    val = dmini_get_string(ctx, "section1", "key2", "");
    TEST_ASSERT(strcmp(val, "value2") == 0, "Section whitespace not trimmed");
    
    dmini_destroy(ctx);
    TEST_PASS();
}

/**
 * @brief Module initialization
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
    DMOD_LOG_INFO("=== DMINI Functionality Tests ===\n\n");
    
    tests_passed = 0;
    tests_failed = 0;
    
    // Run all tests
    test_create_destroy();
    test_parse_string();
    test_set_get();
    test_has_section_key();
    test_remove();
    test_generate_string();
    test_file_io();
    test_comments_whitespace();
    
    // Print summary
    DMOD_LOG_INFO("\n=== Test Summary ===\n");
    DMOD_LOG_INFO("Passed: %d\n", tests_passed);
    DMOD_LOG_INFO("Failed: %d\n", tests_failed);
    
    if (tests_failed == 0)
    {
        DMOD_LOG_INFO("All tests PASSED!\n");
        return 0;
    }
    else
    {
        DMOD_LOG_ERROR("Some tests FAILED!\n");
        return 1;
    }
}

/**
 * @brief Module deinitialization
 */
void dmod_deinit(void)
{
    // Nothing to do
}

#ifndef DMINI_H
#define DMINI_H

#include "dmod.h"

/**
 * @brief DMINI - DMOD INI File Parser Module
 * 
 * This module provides an API for parsing and generating INI files.
 * It uses only SAL (System Abstraction Layer) functions from DMOD.
 * 
 * INI file format:
 * - Sections are defined by [section_name]
 * - Key-value pairs are defined by key=value
 * - Comments start with ; or #
 * - Whitespace is trimmed from keys and values
 */

/**
 * @brief Error codes
 */
#define DMINI_OK                0
#define DMINI_ERR_GENERAL      -1
#define DMINI_ERR_MEMORY       -2
#define DMINI_ERR_INVALID      -3
#define DMINI_ERR_NOT_FOUND    -4
#define DMINI_ERR_FILE         -5

/**
 * @brief INI context type (opaque)
 * 
 * This is an opaque pointer to the INI file context structure.
 */
typedef struct dmini_context* dmini_context_t;

/**
 * @brief Initialize INI context
 * 
 * Creates a new INI context for storing sections and key-value pairs.
 * 
 * @return Pointer to INI context or NULL on error
 */
DMOD_EXPORT dmini_context_t dmini_create(void);

/**
 * @brief Free INI context
 * 
 * Frees all memory associated with the INI context.
 * 
 * @param ctx INI context to free
 */
DMOD_EXPORT void dmini_destroy(dmini_context_t ctx);

/**
 * @brief Parse INI file from string
 * 
 * Parses an INI file from a null-terminated string.
 * 
 * @param ctx INI context
 * @param data String containing INI file contents
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_parse_string(dmini_context_t ctx, const char* data);

/**
 * @brief Parse INI file
 * 
 * Parses an INI file from a file path using SAL file functions.
 * 
 * @param ctx INI context
 * @param filename Path to INI file
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_parse_file(dmini_context_t ctx, const char* filename);

/**
 * @brief Generate INI file to string
 * 
 * Generates an INI file string from the context.
 * The returned string must be freed using Dmod_Free().
 * 
 * @param ctx INI context
 * @return Allocated string containing INI file contents, or NULL on error
 */
DMOD_EXPORT char* dmini_generate_string(dmini_context_t ctx);

/**
 * @brief Generate INI file
 * 
 * Generates an INI file from the context and writes it to a file.
 * 
 * @param ctx INI context
 * @param filename Path to output INI file
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_generate_file(dmini_context_t ctx, const char* filename);

/**
 * @brief Get string value from INI context
 * 
 * Retrieves a string value for the given section and key.
 * 
 * @param ctx INI context
 * @param section Section name (NULL for global section)
 * @param key Key name
 * @param default_value Default value if key not found
 * @return Value string or default_value if not found
 */
DMOD_EXPORT const char* dmini_get_string(dmini_context_t ctx, 
                                          const char* section, 
                                          const char* key, 
                                          const char* default_value);

/**
 * @brief Get integer value from INI context
 * 
 * Retrieves an integer value for the given section and key.
 * 
 * @param ctx INI context
 * @param section Section name (NULL for global section)
 * @param key Key name
 * @param default_value Default value if key not found
 * @return Integer value or default_value if not found
 */
DMOD_EXPORT int dmini_get_int(dmini_context_t ctx, 
                               const char* section, 
                               const char* key, 
                               int default_value);

/**
 * @brief Set string value in INI context
 * 
 * Sets a string value for the given section and key.
 * Creates the section if it doesn't exist.
 * 
 * @param ctx INI context
 * @param section Section name (NULL for global section)
 * @param key Key name
 * @param value Value string
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_set_string(dmini_context_t ctx, 
                                  const char* section, 
                                  const char* key, 
                                  const char* value);

/**
 * @brief Set integer value in INI context
 * 
 * Sets an integer value for the given section and key.
 * Creates the section if it doesn't exist.
 * 
 * @param ctx INI context
 * @param section Section name (NULL for global section)
 * @param key Key name
 * @param value Integer value
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_set_int(dmini_context_t ctx, 
                               const char* section, 
                               const char* key, 
                               int value);

/**
 * @brief Check if section exists
 * 
 * @param ctx INI context
 * @param section Section name
 * @return 1 if section exists, 0 otherwise
 */
DMOD_EXPORT int dmini_has_section(dmini_context_t ctx, const char* section);

/**
 * @brief Check if key exists in section
 * 
 * @param ctx INI context
 * @param section Section name (NULL for global section)
 * @param key Key name
 * @return 1 if key exists, 0 otherwise
 */
DMOD_EXPORT int dmini_has_key(dmini_context_t ctx, 
                               const char* section, 
                               const char* key);

/**
 * @brief Remove section from INI context
 * 
 * @param ctx INI context
 * @param section Section name
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_remove_section(dmini_context_t ctx, const char* section);

/**
 * @brief Remove key from section
 * 
 * @param ctx INI context
 * @param section Section name (NULL for global section)
 * @param key Key name
 * @return DMINI_OK on success, error code on failure
 */
DMOD_EXPORT int dmini_remove_key(dmini_context_t ctx, 
                                  const char* section, 
                                  const char* key);

#endif // DMINI_H

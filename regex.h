#pragma once

#include "types.h"
#include "string/slice.h"

/*
    Current syntax
    +: match 1 or more times
    ?: match 0 or 1 times (optional)
    *: match 0 or more times
   
    Currently only supports individual literal ascii characters.
*/

//!\\[([^\\]]*)\\]\(([^\\)]*)\\)

// Capture groups
// Alphabets

typedef struct regex_node {
    char literal;
    //complex pattern
    bool invert;
    struct regex_node *success;
    struct regex_node *fail;
} regex_node;

typedef struct {
    regex_node *root;
} regex_handle;

typedef struct {
    bool found;
    range_t result_range;
    // u64 capture_count;
    // range_t capture_groups[];
} regex_result;

regex_handle init_regex(const char *pattern);

void regex_debug(regex_handle *handle);

regex_result regex_find_one(regex_handle *handle, string_slice str);
bool regex_find_many(regex_handle *handle, string_slice str, bool (*on_find)(regex_result));
#pragma once

#include "types.h"
#include "string/slice.h"

/*
    Current syntax
    +: match 1 or more times
    ?: match 0 or 1 times (optional)
    *: match 0 or more times
    ^: match opposite of next character
    (: begin capture group
    ): end capture group
   
    Currently only supports individual literal ascii characters. Character ranges [a-zA-Z0-9] are not yet supported, so ^ is used on the next individual character
*/

typedef enum {
    regex_node_match,
    regex_node_start_group,
    regex_node_end_group,
    // TODO: Non-capture groups (?:)
    // TODO: Capture groups need to group things, not just capture
} regex_node_type;

typedef struct regex_node {
    char literal;
    // TODO: Complex pattern
    // TODO: Character ranges [0-9]
    bool invert;
    regex_node_type type;
    struct regex_node *success;
    struct regex_node *fail;
} regex_node;

typedef struct {
    regex_node *root;
} regex_handle;

#define MAX_CAPTURE_GROUPS 16

typedef struct {
    string_slice full_slice;
    bool found;
    range_t result_range;
    u64 capture_count;
    range_t capture_groups[MAX_CAPTURE_GROUPS];
} regex_result;

regex_handle init_regex(const char *pattern);

void regex_debug(regex_handle *handle);

regex_result regex_find_one(regex_handle *handle, string_slice str);
bool regex_find_many(regex_handle *handle, string_slice str, bool (*on_find)(regex_result));
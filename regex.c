#define REGEX_IMPLEMENTATION
#ifdef REGEX_IMPLEMENTATION

#include "regex.h"
#include "syscalls/syscalls.h"
#include "utils/indent.h"
#include "memory/memory.h"

regex_node* regex_new_node(){
    return zalloc(sizeof(regex_node));
}

regex_node* regex_clone_node(regex_node *node){
    regex_node* new_node = regex_new_node();
    memcpy(new_node, node, sizeof(regex_node));
    return new_node;
}

typedef enum { 
    regex_append_invalid, 
    regex_append_success = 1 << 1, 
    regex_append_failure = 1 << 2
} regex_state_append;

#define link_node(node) if (current_append_rule & regex_append_failure) previous_node->fail = node;\
if (current_append_rule & regex_append_success) previous_node->success = node;

// TODO: structure the regex state machine into being stack-based
regex_handle init_regex_slice(string_slice pattern){
    bool ignore_next = false;

    regex_handle handle = {};
    
    regex_node *previous_node = 0;
    regex_state_append current_append_rule = regex_append_success;
    bool should_invert = false;
    for (u64 i = 0; i < pattern.length; i++){
        char next_char = pattern.data[i];
        if (!ignore_next){
            if (next_char == '\\'){
                ignore_next = true;
                continue;
            }
            if (next_char == '*'){
                previous_node->success = previous_node;
                current_append_rule = regex_append_failure;
                continue;
            }
            if (next_char == '+'){
                regex_node *new_node = regex_clone_node(previous_node);
                previous_node->success = new_node;
                new_node->success = new_node;
                previous_node = new_node;
                current_append_rule = regex_append_failure;
                continue;
            }
            if (next_char == '^'){
                should_invert = true;
                continue;
            }
            if (next_char == '?'){
                current_append_rule = regex_append_success | regex_append_failure;
                continue;
            }
            if (next_char == '(' || next_char == ')'){
                regex_node *node = regex_new_node();
                link_node(node);
                node->type = next_char == '(' ? regex_node_start_group : regex_node_end_group;
                current_append_rule = regex_append_success | regex_append_failure;
                previous_node = node;
                continue;
            }
        }
        regex_node *node = regex_new_node();
        node->literal = next_char;
        node->invert = should_invert;
        should_invert = false;
        if (previous_node){
            link_node(node);
        }
        if (!handle.root) {
            handle.root = node;
            // print("Root expects %c",next_char);
        }
        previous_node = node;
        // print("Expecting %c",next_char);
        ignore_next = false;
        current_append_rule = regex_append_success;
    }
    
    return handle;
}

void regex_debug_node(regex_node *node,int depth){
    if (node->type != regex_node_match){
        switch (node->type){
            case regex_node_start_group: 
                print("Start capture group"); break;
            case regex_node_end_group: 
                print("End capture group"); break;
            default: break;
        }
        if (node->success) regex_debug_node(node->success, depth+1);
        else if (node->fail) regex_debug_node(node->fail, depth+1);
        return;
    }
    print("%s%s%c",indent_by(depth),node->invert ? "^" : "",node->literal);
        print("%sSucc:",indent_by(depth));
    if (node->success){
        if (node->success == node) print("%sself",indent_by(depth+1));
        else
            regex_debug_node(node->success, depth+1);
    }
    print("%sFail:",indent_by(depth));
    if (node->fail){
        if (node->fail == node) print("%sself",indent_by(depth+1));
        else
            regex_debug_node(node->fail, depth+1);
    }
}

void regex_debug(regex_handle *handle){
    if (!handle) {
        print("No handle");
        return;
    }
    if (!handle->root){
        print("No root");
        return;
    }
    regex_node *current_node = handle->root;
    regex_debug_node(current_node,0);
}

regex_handle init_regex(const char *pattern){
    return init_regex_slice(slice_from_literal(pattern));
}

regex_result find_one_result = {};

bool regex_find_one_handler(regex_result result){
    find_one_result = result;
    return false;
}

regex_result regex_find_one(regex_handle *handle, string_slice str){
    find_one_result = (regex_result){};
    regex_find_many(handle, str, regex_find_one_handler);
    return find_one_result;
}

static inline bool regex_does_match(char c, regex_node *node){
    return node->invert ^ (node->literal == c);
}

bool regex_find_many(regex_handle *handle, string_slice str, bool (*on_find)(regex_result)){
    if (!handle || !handle->root || !str.length)        
        return false;
    regex_node *current_node = 0;
    regex_result result = {.full_slice = str};
    bool ever_found = false;
    result.found = false;
    bool is_capture_group = false;
    for (u64 i = 0; i < str.length; i++){
        if (!current_node) current_node = handle->root;

        if (current_node->type != regex_node_match){
            switch (current_node->type){
                case regex_node_start_group:
                    if (is_capture_group){
                        print("[REGEX implementation error] nesting capture groups not allowed");
                        return false;
                    }
                    if (result.capture_count >= MAX_CAPTURE_GROUPS){
                        print("[REGEX implementation error] there is a maximum of %i capture groups",MAX_CAPTURE_GROUPS);
                        return false;
                    }
                    is_capture_group = true;
                    result.capture_count++;
                    result.capture_groups[result.capture_count].start = i;
                    current_node = current_node->success ?: current_node->fail;
                    i--;
                    continue;
                case regex_node_end_group:
                    if (!is_capture_group){
                        print("[REGEX error] ending unknown capture group");
                        return false;
                    }
                    is_capture_group = false;
                    current_node = current_node->success ?: current_node->fail;
                    i--;
                    continue;
                    break;
                default:
                    break;
            }
            continue;
        }
        
        char current_char = str.data[i];
        // print("%c vs %c",current_node->literal,current_char);
        
        if (regex_does_match(current_char, current_node)){
            if (is_capture_group){
                result.capture_groups[result.capture_count].size++;
            }
            if (current_node == handle->root) result.result_range.start = i;
            if (current_node->success) current_node = current_node->success;
            else {
                ever_found = true;
                result.found = true;
                result.result_range.size = i-result.result_range.start;
                result.full_slice = str;
                if (!on_find(result)) break;
                result = (regex_result){};
                continue;
            }
        } else if (current_node != handle->root) {
            i--;
            if (current_node->fail) current_node = current_node->fail;
            else {
                // print("Found %c instead of expected %c",current_char, current_node->literal);
                current_node = 0;
                result = (regex_result){};
                continue;
            }
        }
    }
    return ever_found;
}

#endif
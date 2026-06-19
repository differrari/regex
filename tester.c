#include "syscalls/syscalls.h"
#include "regex.h"

typedef struct {
    char *literal;
    bool should_match;
} regex_test;

regex_test tests[] = {
    {"![hello](category)", true},
    {"![hello](category/other category/something else)", true},
    {"[hello", false},
    // {"![hello]\\", true},
    // {"![hell8o]\\", true},
    // {"![hell88o]\\", false},
    // {"![heo]\\", false},
    // {"![heo]", false},
    // {".", false},
    // {"", false},
    // {"![text](category/other)",false}
};

int main(){

    // !\[[^\]*]\]\([^\)]*\)
    // !\[([^\]*)]\]\(([^\)]*)\)
    
    print("Hello world");
    regex_handle handle = init_regex("!\\[(^\\]*)\\]\\((^\\)*)\\)");
    regex_debug(&handle);
    for (int i = 0; i < N_ARR(tests); i++){
        regex_result result = regex_find_one(&handle, slice_from_literal(tests[i].literal));
        if (result.found != tests[i].should_match){
            print("Wrong result in test %i. Expected %i got %i",i,tests[i].should_match,result.found);
            return -1;
        } else if (tests[i].should_match && result.capture_count > 0){
            print("Capture groups for test %i",i);
            for (int j = 1; j <= result.capture_count; j++){
                print("\t[%i]: {%i,%i}",j,result.capture_groups[j].start,result.capture_groups[j].size);
            }
        }
    }
    print("Test passed");
    return 0;
}
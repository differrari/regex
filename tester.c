#include "syscalls/syscalls.h"
#include "regex.h"

typedef struct {
    char *literal;
    bool should_match;
} regex_test;

regex_test tests[] = {
    {"![hello]\\", true},
    {"![hell8o]\\", true},
    {"![hell88o]\\", false},
    {"![heo]\\", false},
    {"![heo]", false},
    {".", false},
    {"", false}
};

int main(){
    print("Hello world");
    regex_handle handle = init_regex("!\\[hel+8?o\\]\\\\");
    regex_debug(&handle);
    for (int i = 0; i < N_ARR(tests); i++){
        regex_result result = regex_find_one(&handle, slice_from_literal(tests[i].literal));
        if (result.found != tests[i].should_match){
            print("Wrong result in test %i. Expected %i got %i",i,tests[i].should_match,result.found);
            return -1;
        }
    }
    print("Test passed");
    return 0;
}
#include "redbuild.h"
#include "syscalls/syscalls.h"
bool regex(char* source_file){
	new_module("regex");
	if (source_file){
		set_name("regex_demo");
	} else {
		set_name("regex");
	}
	add_precomp_flag("REGEX_IMPLEMENTATION");
	if (source_file){
		set_package_type(package_bin);
	} else {
		set_package_type(package_lib);
	}
	set_target(target_native, false);
	source("regex.c");
	if (source_file){
		source(source_file);
	}
	
	if (compile()){
		gen_compile_commands(source_file);
		if (source_file){
			return run();
		}
		
		return true;
	}
	
	return false;
}

int main(int argc, strarr argv){
	int __return_val;
	parse_arguments(argc, argv);
	rebuild_self(true);
	__return_val = !regex("tester.c");
	goto defer;
	defer:
	emit_compile_commands();
	return __return_val;
}

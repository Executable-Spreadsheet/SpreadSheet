#define SB_IMPL
#include "sb.h"

int main(int argc, char* argv[]) {
    sb_BUILD(argc, argv) {
        sb_mkdir("build");
        sb_target_dir("build/");
        sb_EXEC() {
             
            sb_add_flag("g");

            sb_add_include_path("include");

            sb_add_file("src/main.c");
            sb_add_file("src/util.c");
            
            sb_set_out("app");
            sb_export_command();

            if (sb_check_arg("inc")) {
                sb_set_find_deps();
                sb_set_incremental();
            }
        }
        if (sb_check_arg("test")) {
            sb_fence();
            sb_CMD() {
                sb_cmd_main("clear");
            }

            sb_fence();
            sb_CMD() {
                sb_cmd_arg("./build/app");
            }
        }
    }
}

#include <util/util.h>
#include <unistd.h>


int main() {
    log("%n", ttyname(STDOUT_FILENO));
}


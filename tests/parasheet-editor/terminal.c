#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <util/util.h>

int main() {
	char buf[PATH_MAX + 1] = {0};
	u32 i = getppid();
	log("%d", i);

	char path[PATH_MAX + 1] = {0};
	snprintf(path, PATH_MAX, "/proc/%d/stat", i);

	FILE* s = fopen(path, "r");

	(void)fread(buf, PATH_MAX, 1, s);

	strtok(buf, " ");
	strtok(NULL, " ");
	strtok(NULL, " ");
	char* id = strtok(NULL, " ");

	char name[PATH_MAX + 1] = {0};
	snprintf(path, PATH_MAX, "/proc/%s/exe", id);

	(void)readlink(path, name, PATH_MAX);

	log("%n", name);
}

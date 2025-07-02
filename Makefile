# Note(ELI): Here is an alternative build method.
# It should be equivalent to sb_build
# if you want it. I'll probably ask Quincy to
# write this, but I can throw together something
# quick

# Currently this just builds the whole project in
# one go, so no incremental stuff yet.

# At some point we'll probably also want something
# which generates a compile_commands.json file.

LDFLAGS := -Iinclude
CFLAGS := -g


build/app : src/main.c src/util.c
	mkdir -p build/
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONEY: clean

clean:
	rm -rf build/

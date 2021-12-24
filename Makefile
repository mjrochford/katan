BUILD_DIR = ./build

.PHONY: run init test test-debug format clean

run: init
	meson compile -C ${BUILD_DIR} && ./build/katan

test: init
	meson test -C ${BUILD_DIR}

test-debug: init
	meson test --gdb -C ${BUILD_DIR}

init:
	@test -d build || meson ${BUILD_DIR}

format:
	git ls-files | rg ".c|.h" | xargs clang-format -i

clean:
	rm -rf ${BUILD_DIR}

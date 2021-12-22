BUILD_DIR = ./build

run:
	meson compile -C ${BUILD_DIR} && ./build/katan

test:
	meson test -C ${BUILD_DIR}
test-debug:
	meson test --gdb -C ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

build:
	# Cleans the build directory and rebuilds the project, SHOULD FIX THE LSP,
	# also always run it as make -B build
	rm -rf build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build -S . && cmake --build ./build

run:
	./build/OlympusObservatory

fix_lsp:
	make build && cd build/ && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../CMakeLists.txt


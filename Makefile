dev:
	cmake --build build
	./docker.sh
	make run

clean:
	rm -rf build

run:
	./build/OlympusObservatory

fix_lsp:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build -S .


dev:
	cmake --build build
	./docker.sh
	sleep 2
	make run

t:
	cmake --build build

clean:
	rm -rf build

run:
	./build/OlympusObservatory

fix_lsp:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build -S .


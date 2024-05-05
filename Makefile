build: 
	rm -rf build && cmake -B build -S . && cmake --build build -j32

run:
	./build/OlympusObservatory

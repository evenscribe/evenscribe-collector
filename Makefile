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


spawn_db:
	docker run --rm -d --name victorialogs -p 9428:9428 -v /tmp/victoria-logs-data:/victoria-logs-data \
	docker.io/victoriametrics/victoria-logs:v0.19.0-victorialogs

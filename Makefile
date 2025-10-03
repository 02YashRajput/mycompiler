.PHONY: clean build run docker-build docker-run docker-exec docker-clean

clean:
	rm -rf build

build: clean
	mkdir -p build
	cd build && cmake ..

run:
	cmake --build build/
	./build/mycompiler input.txt
	./out

docker-run:
	docker-compose up -d

docker-exec: docker-run
	docker exec -it mycompiler_dev /bin/bash

docker-clean:
	docker-compose down -v --rmi all --remove-orphans
	

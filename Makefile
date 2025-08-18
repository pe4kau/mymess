
.PHONY: dev prod build test clean

dev:
	docker compose up --build

prod:
	docker compose -f docker-compose.yml -f docker-compose.prod.yml up -d --build

build:
	cmake -B build -S . && cmake --build build -j

test:
	ctest --test-dir build --output-on-failure || true

clean:
	rm -rf build

.PHONY: all test clean editor client common server build coverage gcovr streets

COVERAGE_DIR := build
GCOVR_DIR := build/tools
# Usa el gcovr del sistema si existe; si no, uno local en $(GCOVR_DIR) (lo instala el target gcovr)
GCOVR ?= $(if $(shell command -v gcovr),gcovr,PYTHONPATH=$(GCOVR_DIR) $(GCOVR_DIR)/bin/gcovr)

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug -DNFS_COVERAGE=OFF $(EXTRA_GENERATE)
	cmake --build  build/ $(EXTRA_COMPILE)

run-tests: compile-debug
	./build/nfs_tests
	QT_QPA_PLATFORM=offscreen ./build/nfs_client_tests

gcovr:
	@command -v gcovr >/dev/null || test -x $(GCOVR_DIR)/bin/gcovr || \
		pip3 install --quiet --target $(GCOVR_DIR) gcovr

test: coverage

coverage: gcovr
	mkdir -p $(COVERAGE_DIR)/
	cmake -S . -B ./$(COVERAGE_DIR) -DCMAKE_BUILD_TYPE=Debug -DNFS_COVERAGE=ON $(EXTRA_GENERATE)
	cmake --build $(COVERAGE_DIR)/ $(EXTRA_COMPILE)
	find $(COVERAGE_DIR)/ -name '*.gcda' -delete
	./$(COVERAGE_DIR)/nfs_tests
	QT_QPA_PLATFORM=offscreen ./$(COVERAGE_DIR)/nfs_client_tests
	mkdir -p $(COVERAGE_DIR)/report
	$(GCOVR) --root . --object-directory $(COVERAGE_DIR) \
		--filter 'client/' --filter 'server/' --filter 'common/' --filter 'editor/' \
		--print-summary --html-details $(COVERAGE_DIR)/report/index.html

# Regenera las grillas de calles de server/world/streets/ desde las máscaras de server/tools/streets/
STREETS_CELL_SIZE := 16
STREETS_CITIES := liberty_city san_andreas vice_city

streets:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build build/ --target nfs_streets_generator
	mkdir -p server/world/streets
	for city in $(STREETS_CITIES); do \
		./build/nfs_streets_generator server/tools/streets/masks/streets_mask_$$city.png \
			server/world/streets/streets_$$city.json $(STREETS_CELL_SIZE) || exit 1; \
	done

all: clean run-tests

clean:
	rm -Rf build/

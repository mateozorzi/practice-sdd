.PHONY: all test clean editor client common server build coverage gcovr

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

all: clean run-tests

clean:
	rm -Rf build/

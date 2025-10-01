# Makefile
BUILD_DIR := build

.PHONY: all clean configure build

all: clean configure build

clean:
	@echo ">> Xoá thư mục $(BUILD_DIR)"
	rm -rf $(BUILD_DIR)

configure:
	@echo ">> Tạo thư mục $(BUILD_DIR) và chạy cmake .."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..

build:
	@echo ">> Biên dịch dự án"
	cmake --build $(BUILD_DIR) -j$(shell nproc)

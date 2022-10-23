LLVM_DIR=/usr/local/llvm10ra/
all: build
build: ast-interpreter
	cd build && make -j
ast-interpreter:
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_DIR=$(LLVM_DIR) ..
clean:
	rm -rf build && mkdir build
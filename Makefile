all:
	mkdir build && cd build && cmake .. --preset apple-mac && cmake --build . 

clean:
	rm -rf build

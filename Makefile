all:
	mkdir -p bin
	g++ -shared -fPIC -o bin/libFile.so basic_fs_disk.cpp
	g++ -shared -fPIC -o bin/libMems.so basic_fs_mem.cpp -lrt
	g++ -rdynamic -o bin/create_disk.out create_disk.cpp filesystem.cpp -ldl -lrt
	g++ -rdynamic -o bin/add_file.out add_file.cpp filesystem.cpp -ldl -lrt
	g++ -rdynamic -o bin/get_file.out get_file.cpp filesystem.cpp -ldl -lrt
	g++ -rdynamic -o bin/stats.out stats.cpp filesystem.cpp -ldl -lrt

all:
	mkdir -p out
	g++ --std=c++11 -Wall -lpthread -shared -fPIC -c src/ThreadPool.cpp -o out/libatomy.so
test:
	mkdir -p bin
	g++ --std=c++11 -Wall -I. -Lout -lpthread -latomy -o bin/test testing/main.cpp
clean:
	rm -rf out bin
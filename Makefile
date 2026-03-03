OBJECTS := cmem.o
OBJECTS += compat.o
OBJECTS += encoding_check.o
OBJECTS += get_encoding.o
OBJECTS += ofstream.o

linux: chario.a

windows: chario.lib

chario.a: $(OBJECTS)
	gcc-ar rcs $@ $^

chario.lib: $(OBJECTS)
	gcc-ar rcs $@ $^

%.o: src/%.cpp
	g++ -c $^ -o $@ -I./chario -I./dependencies --std=c++20

clean:
	rm -f *.o

TARGET=temp_name

CC=g++
CFLAGS=-std=c++11 -Wall -O3 -Ilibs/include -fopenmp
LDFLAGS=-lgomp -Llibs/lib -lvoro++

LIB_VORO=libs/lib/libvoro++.a

SRC=$(addprefix	src/,\
		main.cpp interpolation.cpp power_diagram.cpp image.cpp pixel.cpp stb_implem.cpp)

OBJ=$(patsubst src/%.cpp, build/%.o, $(SRC))

all: libs $(TARGET)

libs: $(LIB_VORO)

$(LIB_VORO):
	cd voro++-0.4.6 && make && make install

build:
	mkdir -p build

$(TARGET): build $(OBJ)
	$(CC) -o $@	$(OBJ) $(LDFLAGS)

build/%.o: src/%.cpp
	$(CC) -o $@	-c $< $(CFLAGS)

clean:
	rm -rf build

cleanlib:
	rm -rf libs
	cd voro++-0.4.6 && make clean

mrproper: clean cleanlib
	rm -f $(TARGET)

.PHONY:	all	clean mrproper

all: 
	make -C ./src/

clean:
	make clean -C ./src/

build:
	make build -C ./src/

tsocks:
	make tsocks -C ./src/
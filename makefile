usage:
	@echo "Usage:"
	@echo "'make build': builds the project at folder ./lib"
build:
	@[ -d "./lib/" ]&&rm -rf ./lib
	@mkdir lib
	@echo "Building libmiceapi.so"
	gcc -c -fpic -o ./lib/miceapi_main.o ./src/miceapi_main.c
	gcc -c -fpic -o ./lib/miceapi_events.o ./src/miceapi_events.c
	gcc -shared -o ./lib/libmiceapi.so ./lib/miceapi_main.o ./lib/miceapi_events.o
	@rm ./lib/miceapi_main.o
	@rm ./lib/miceapi_events.o
	@echo "Done!"
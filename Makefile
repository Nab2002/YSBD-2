ht:
	@echo " Compile ht_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/neo_hash_file.c -lbf -o ./build/runner_ht -O2

bf:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner_bf -O2


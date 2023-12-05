ht:
	@echo " Compile ht_main with debugging information..."
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/neo_hash_file.c -lbf -o ./build/runner_ht -O2

bf:
	@echo " Compile bf_main with debugging information..."
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner_bf -O2

valgrind_ht:
	@echo "Run ht_main with Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all ./build/runner_ht

valgrind_bf:
	@echo "Run bf_main with Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all ./build/runner_bf
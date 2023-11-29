#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hash_file.h"

#define RECORDS_NUM 1        // you can change it if you want
#define GLOBAL_DEPT 1           // initial global depth = 1
#define FILE_NAME "data.db"

void *fileArray;

const char* names[] = {
  "Yannis",
  "Christofos",
  "Sofia",
  "Marianna",
  "Vagelis",
  "Maria",
  "Iosif",
  "Dionisis",
  "Konstantina",
  "Theofilos",
  "Giorgos",
  "Dimitris"
};

const char* surnames[] = {
  "Ioannidis",
  "Svingos",
  "Karvounari",
  "Rezkalla",
  "Nikolopoulos",
  "Berreta",
  "Koronis",
  "Gaitanis",
  "Oikonomou",
  "Mailis",
  "Michas",
  "Halatsis"
};

const char* cities[] = {
  "Athens",
  "San Francisco",
  "Los Angeles",
  "Amsterdam",
  "London",
  "New York",
  "Tokyo",
  "Hong Kong",
  "Munich",
  "Miami"
};

#define CALL_OR_DIE(call)     \
  {                           \
    HT_ErrorCode code = call; \
    if (code != HT_OK) {      \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);
  
  
  //
  printf("size of Record: %ld\n", sizeof(Record));
  printf("size of block: %d\n", BF_BLOCK_SIZE);
  printf("maximum number of records in each block: %ld\nstorage left: %ld\n\n\n", BF_BLOCK_SIZE/sizeof(Record), BF_BLOCK_SIZE - (BF_BLOCK_SIZE/sizeof(Record))*sizeof(Record));
  //

  CALL_OR_DIE(HT_Init());

  int indexDesc;
  CALL_OR_DIE(HT_CreateIndex(FILE_NAME, GLOBAL_DEPT));
  printf("main: HT_CreateIndex: OK\n");
  // CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc)); 
  printf("main: HT_OpenIndex: OK\nIndex: %d\n\n", indexDesc);
  

  Record record;
  
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    // create a record
    record.id = id;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);

    printf("\n\n%dh klhsh:\n", record.id+1);
    // CALL_OR_DIE(HT_InsertEntry(indexDesc, record));
  }
  printf("\n\nmain: HT_InsertEntries: OK\n");

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  // CALL_OR_DIE(HT_PrintAllEntries(indexDesc, &id));

  // CALL_OR_DIE(HT_CloseFile(indexDesc));
  printf("main: HT_CloseFile: OK\n");
  
  BF_Close();
}

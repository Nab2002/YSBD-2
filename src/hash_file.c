#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20


#define TABLE_SIZE 3
#define HP_ERROR -1

#define HT_OK 0
#define HT_ERROR -1

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}


OpenFileInfo openFiles[MAX_OPEN_FILES];     // Array to store open files information
int nextAvailableIndex = 0;                 // Index to track the next available slot in the array




HT_ErrorCode HT_Init() {
  return HT_OK;
}


/* * * * * * * HT_CreateIndex * * * * * * */
HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  for (int i = 0; i < MAX_OPEN_FILES; ++i) {
    openFiles[i].fd = -1;         // Initialize to a default value
    openFiles[i].filename[0] = '\0'; // Initialize to an empty string
    openFiles[i].hashTable = NULL;
  }
  
  // Ελέγχουμε αν το αρχείο υπάρχει ήδη.
  BF_ErrorCode code = BF_CreateFile(filename);
  if (code == BF_FILE_ALREADY_EXISTS) {
    printf("ERROR. The file you are trying to create already exists!\n\n");
    return HT_ERROR;
  }

  // Ανοίγουμε το αρχείο και επιστρέφουμε το file descriptor του στην μεταβλητή fd.
  int fd;
  CALL_BF(BF_OpenFile(filename, &fd));

  // Ορίζουμε, αρχικοποιούμε και δεσμεύουμε ένα block, για τα μεταδεδομένα του αρχείου.
  BF_Block *infoBlock;
  BF_Block_Init(&infoBlock);
  CALL_BF(BF_AllocateBlock(fd, infoBlock));

  HT_Info info;

  // Ορίζουμε έναν δείκτη που δείχνει στα δεδομένα του block των μεταδεδομένων.
  void *infoData = BF_Block_GetData(infoBlock);
        
  strncpy(info.fileType, "Hash File", sizeof(info.fileType));
  info.fileType[sizeof(info.fileType) - 1] = '\0';  // Ensure null-termination
  
  strncpy(info.fileName, filename, sizeof(info.fileName));
  info.fileName[sizeof(info.fileName) - 1] = '\0';  // Ensure null-termination

  strncpy(info.hash_field, "id", sizeof(info.hash_field));
  info.hash_field[sizeof(info.hash_field) - 1] = '\0';  // Ensure null-termination

  info.fileDesc = fd;
  info.indexDesc = nextAvailableIndex;
  info.total_num_of_recs = 0;
  info.num_of_blocks = 1;
  info.HT_Info_size = sizeof(HT_Info);
  info.globalDepth = depth;

  

  memcpy(infoData, &info, sizeof(HT_Info));
  HT_PrintMetadata(infoData);

  // Set the pointers in OpenFileInfo to point to the corresponding blocks
  // openFiles[nextAvailableIndex].firstBlock = infoData;
  strncpy(openFiles[nextAvailableIndex].filename, filename, sizeof(info.fileName) - 1);
  openFiles[nextAvailableIndex].fd = info.fileDesc;


  if(infoBlock != NULL) {
    BF_Block_SetDirty(infoBlock);
    CALL_BF(BF_UnpinBlock(infoBlock));
    BF_Block_Destroy(&infoBlock);
  }
  

  CALL_BF(BF_CloseFile(fd));

  // openFiles[nextAvailableIndex].hashTable->global_depth = depth;
  nextAvailableIndex ++;
  return HT_OK;
}


/* * * * * * * HT_OpenIndex * * * * * * */
HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  // Αρχικοποιούμε τα περιεχόμενα του δείκτη.
  *indexDesc = -1;
  BF_ErrorCode error;
  for(int i = 0; i < MAX_OPEN_FILES; i++) {
    // Συγκρίνουμε το όνομα που μας δόθηκε με τα ονόματα των ανοικτών αρχείων στον πίνακα.
    if(strcmp(openFiles[i].filename, fileName) == 0) {
      error = BF_OpenFile(fileName, &openFiles[i].fd);
      *indexDesc = i;
    }
  }
  if(error != BF_OK) {
    printf("Error! The file could not be opened!\n");
    return HT_ERROR;
  }
  printf("\n\nHT_OpenIndex: openFiles[%d].fd: %d\n", *indexDesc, openFiles[*indexDesc].fd);


  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  if (indexDesc < 0 || indexDesc >= MAX_OPEN_FILES || openFiles[indexDesc].fd == -1) {
    return HT_ERROR;
  }

  BF_ErrorCode code = BF_CloseFile(openFiles[indexDesc].fd);
  if (code != BF_OK) {
    // Handle file close error
    return HT_ERROR;
  }

  free(openFiles[indexDesc].hashTable);  // Free the hash table memory

  // Optionally, reset the file descriptor and other fields
  openFiles[indexDesc].fd = -1;
  openFiles[indexDesc].hashTable = NULL;

  return HT_OK;
}

HT_ErrorCode HT_Spit_Bucket (int indexDesc, Record record, void *oldBucketData, int *hashValue) {
  printf("Den xwraei\n");  
  printf("Eggrafh pros eisagwgh:\nhash: %d, ID: %d, name: %s, surname: %s, city: %s\n\nPalies eggrafes:\n", *hashValue, record.id, record.name, record.surname, record.city);

  BF_Block *newBucket;
  BF_Block* infoBlock;
  BF_Block_Init(&infoBlock);

  HT_Info *info = (HT_Info *)BF_Block_GetData(infoBlock);


  info->globalDepth ++;
  Block_Info *oldBucket_ptr;
  Block_Info *newBucket_ptr;

  oldBucket_ptr->local_depth ++;
  HashTable_resize(&openFiles[indexDesc].hashTable, info);


  BF_Block_Init(&newBucket);
  CALL_BF(BF_AllocateBlock(info->fileDesc, newBucket));

  void *newBucketData = BF_Block_GetData(newBucket);

  CALL_BF(BF_GetBlockCounter(info->fileDesc, &info->num_of_blocks));
  newBucket_ptr->block_id = info->num_of_blocks - 1;
  openFiles[indexDesc].hashTable[*hashValue] = newBucket_ptr->block_id;
  printf("\nopenFiles[indexDesc].hashTable[hashValue]: %d\n\n", openFiles[indexDesc].hashTable[*hashValue]);
  newBucket_ptr->bucket_size = 0;
  newBucket_ptr->local_depth = oldBucket_ptr->local_depth;
  newBucket_ptr->hashvalue = (oldBucket_ptr->hashvalue << 1) + 1;

  printf("\n\nBlock Info struct:\n");
  printf("blockInfo.block_id: %d\n", newBucket_ptr->block_id);
  printf("blockInfo.bucket_size: %d\n", newBucket_ptr->bucket_size);
  printf("sizeof(blockInfo): %ld\n\n", sizeof(newBucket_ptr));

  
  memcpy(newBucket, newBucket_ptr, sizeof(Block_Info));
  
  if(openFiles[indexDesc].hashTable[newBucket_ptr->hashvalue] == -1)
    openFiles[indexDesc].hashTable[newBucket_ptr->hashvalue] = newBucket_ptr->block_id;
  // for() {

  // }
  printf("%d\n", newBucket_ptr->block_id);
  Record *records;
  Block_Info *currentBucket;
  void *data;
  int temp = oldBucket_ptr->bucket_size + 1;
  int br = 0;
  for(int i = 0; i < temp; i++) {
    if(newBucket_ptr->bucket_size > 0) {
      // Επανατοποθετούμε μία-μία τις εγγραφές στον κατάλληλο κάδο.
      records = (Record *)((char *)oldBucketData + sizeof(Block_Info) + sizeof(Record) * i);
      printf("\n\nBEFORE hashValue: %d\n", *hashValue);
      *hashValue = hash(records->id, newBucket_ptr->local_depth);
      printf("AFTER hashValue: %d\n\n", *hashValue);
      newBucket_ptr->bucket_size --;
    }
    else {
      // Μόλις τελειώσουμε με τις ήδη υπάρχουσες εγγραφές,
      // εξετάζουμε την τιμή κατακερματισμού της δοθείσας.
      *hashValue = hash(record.id, newBucket_ptr->local_depth);
      records = &record;
      info->total_num_of_recs ++;
    }

    // Αναλόγως το hash value της εγγραφής οι δείκτες data και currentDirectory δείχνουν
    // στα δεδομένα του bucket και του directory στα οποία θα καταλήξει η εγγραφή.
    if(*hashValue == oldBucket_ptr->hashvalue && oldBucket_ptr->bucket_size * sizeof(Record) < BF_BLOCK_SIZE) {
      data = oldBucketData;
      currentBucket = oldBucket_ptr;
    }
    else if(*hashValue == newBucket_ptr->hashvalue && newBucket_ptr->bucket_size * sizeof(Record) < BF_BLOCK_SIZE){ 
      data = newBucketData;
      currentBucket = newBucket_ptr;
    }
    else {
      BF_Block_SetDirty(newBucket);
      
      CALL_BF(BF_UnpinBlock(newBucket));
      BF_Block_Destroy(&newBucket);

      CALL_BF(BF_UnpinBlock(infoBlock));
      BF_Block_Destroy(&infoBlock);
      printf("skata\n");
      br = 1;
      break;
    }
    Record *currentRecord = (Record *)((char *)data + sizeof(Record) * currentBucket->bucket_size);
    memcpy(currentRecord, records, sizeof(Record));
    printf("hash: %d, ID: %d, name: %s, surname: %s, city: %s\n", *hashValue, records->id, records->name, records->surname, records->city);
    printf("Eggrafes mexri stigmhs sto bucket me id %d: %d\n\n", currentBucket->block_id, currentBucket->bucket_size+1);
  }

  if(br) 
    CALL_BF(HT_Spit_Bucket (indexDesc, record, oldBucketData, hashValue));
  
  currentBucket->bucket_size ++;
  BF_Block_SetDirty(newBucket);
  CALL_BF(BF_UnpinBlock(newBucket));
  BF_Block_Destroy(&newBucket);
  return HT_OK;
}



/* * * * * * * * * HT_InsertEntry * * * * * * * * */
HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  // Φορτώνουμε το block με τα μεταδεδομένα του αρχείου στην buffer.
  BF_Block* infoBlock;
  BF_Block_Init(&infoBlock);
  
  printf("HT_InsertEntry: openFiles[%d].fd: %d\n\n", indexDesc, openFiles[indexDesc].fd);
  CALL_BF(BF_GetBlock(openFiles[indexDesc].fd, 0, infoBlock));


  HT_Info *info = (HT_Info *)BF_Block_GetData(infoBlock);
  // HT_PrintMetadata(info);


  
  // Υπολογίζουμε την τιμή κατακερματισμού του εκάστοτε record.id.
  int hashValue = hash(record.id, info->globalDepth);
  
  HashTable_resize(&openFiles[indexDesc].hashTable, info);

  BF_Block *bucket;
  BF_Block *newBucket;
  Block_Info blockInfo;
  void * bucketData;

  // i = index του hashTable
  // hashTable[i] == bucket_id
  if(openFiles[indexDesc].hashTable[hashValue] == -1) {
    BF_Block_Init(&bucket);
    CALL_BF(BF_AllocateBlock(info->fileDesc, bucket));

    bucketData = BF_Block_GetData(bucket);

    CALL_BF(BF_GetBlockCounter(info->fileDesc, &info->num_of_blocks));
    blockInfo.block_id = info->num_of_blocks - 1;
    openFiles[indexDesc].hashTable[hashValue] = blockInfo.block_id;
    printf("\nopenFiles[indexDesc].hashTable[hashValue]: %d\n\n", openFiles[indexDesc].hashTable[hashValue]);
    blockInfo.bucket_size = 0;
    blockInfo.local_depth = info->globalDepth;
    blockInfo.hashvalue = hashValue;

    printf("\n\nBlock Info struct:\n");
    printf("blockInfo.block_id: %d\n", blockInfo.block_id);
    printf("blockInfo.bucket_size: %d\n", blockInfo.bucket_size);
    printf("sizeof(blockInfo): %ld\n\n", sizeof(blockInfo));

    
    memcpy(bucketData, &blockInfo, sizeof(Block_Info));
    
  }
  else {
    BF_Block_Init(&bucket);
    CALL_BF(BF_GetBlock(info->fileDesc, openFiles[indexDesc].hashTable[hashValue], bucket));
    bucketData = BF_Block_GetData(bucket);
  }
  Block_Info *ptr = bucketData;
  Record *recordinBlock = (Record *)(bucketData + sizeof(Block_Info) + (ptr->bucket_size * sizeof(Record)));
  
  // Αντιγράφουμε τα δεδομένα του record στα περιεχόμενα του δείκτη *recordinBlock,
  // που δείχνει στην διεύθυνση της επόμενης εγγραφής.
  recordinBlock->id = record.id;

  strncpy(recordinBlock->name, record.name, sizeof(recordinBlock->name));
  recordinBlock->name[sizeof(recordinBlock->name) - 1] = '\0';  // Ensure null-termination


  strncpy(recordinBlock->surname, record.surname, sizeof(recordinBlock->surname));
  recordinBlock->surname[sizeof(recordinBlock->surname) - 1] = '\0';  // Ensure null-termination

  strncpy(recordinBlock->city, record.city, sizeof(recordinBlock->city));
  recordinBlock->city[sizeof(recordinBlock->city) - 1] = '\0';  // Ensure null-termination
  
  if(BF_BLOCK_SIZE > sizeof(Block_Info) + (ptr->bucket_size + 1) * sizeof(Record)) {
    printf("\n\n&bucketData = %p\n", bucketData);
    printf("&recordinBlock = %p\n", recordinBlock);


    printf("Record data: ID = %d, Name = %s, Surname = %s, City = %s\n", record.id, record.name, record.surname, record.city);

    ptr->bucket_size ++;
    info->total_num_of_recs ++;
    printf("hash: %d, ID: %d, name: %s, surname: %s, city: %s\n", hashValue, recordinBlock->id, recordinBlock->name, recordinBlock->surname, recordinBlock->city);
    printf("Eggrafes mexri stigmhs sto bucket %d: %d\n\n", hashValue, ptr->bucket_size);
    
    CALL_BF(BF_GetBlockCounter(info->fileDesc, &info->num_of_blocks));
    
    BF_Block_SetDirty(bucket);
    BF_Block_SetDirty(infoBlock);
  
  }
  else if(ptr->local_depth == info->globalDepth) {// Σε αυτήν την περίπτωση η εγγραφή δεν χωράει στο block.
    // (1) Διπλασιάζουμε τον πίνακα κατακερματισμού.
    printf("Den xwraei\n");
    
    printf("Eggrafh pros eisagwgh:\nhash: %d, ID: %d, name: %s, surname: %s, city: %s\n\nPalies eggrafes:\n", hashValue, record.id, record.name, record.surname, record.city);
    CALL_BF(HT_Spit_Bucket (indexDesc, record, bucketData, &hashValue));
    
    BF_Block_SetDirty(bucket);
  }
  CALL_BF(BF_UnpinBlock(bucket));
  BF_Block_Destroy(&bucket);

  CALL_BF(BF_UnpinBlock(infoBlock));
  BF_Block_Destroy(&infoBlock);
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
// Load necessary blocks into the buffer

    // // Load the block with metadata information
    // BF_Block *infoBlock;
    // BF_Block_Init(&infoBlock);
    // CALL_BF(BF_GetBlock(openFiles[indexDesc].info->fileDesc, 0, infoBlock));
    // void *infoData = BF_Block_GetData(infoBlock);
    // HT_Info *info = (HT_Info *)infoData;

    // // Load the block with hash table information
    // BF_Block *hashTableBlock;
    // BF_Block_Init(&hashTableBlock);
    // CALL_BF(BF_GetBlock(openFiles[indexDesc].info->fileDesc, 1, hashTableBlock));
    // void *hashTableData = BF_Block_GetData(hashTableBlock);
    // HashTable *hashTable = (HashTable *)hashTableData;

    // // Load the block with directory information
    // BF_Block *directoryBlock;
    // BF_Block_Init(&directoryBlock);
    // CALL_BF(BF_GetBlock(openFiles[indexDesc].info->fileDesc, 2, directoryBlock));
    // void *directoryData = BF_Block_GetData(directoryBlock);
    // Directory *directory = (Directory *)directoryData;

    // // Find the bucket that corresponds to the given id
    // int hashValue = hash(*id, 4); // Assuming the same hash function is used
    // Bucket *targetBucket = NULL;

    // for (int i = 0; i - 2 < directory->num_of_buckets; i + 2) {
    //     BF_Block *bucketBlock;
    //     BF_Block_Init(&bucketBlock);
    //     CALL_BF(BF_GetBlock(openFiles[indexDesc].info->fileDesc, 3 + i, bucketBlock));
    //     void *bucketData = BF_Block_GetData(bucketBlock);
    //     Bucket *currentBucket = (Bucket *)bucketData;

    //     if (currentBucket->hash_value == hashValue) {
    //         targetBucket = currentBucket;
    //         BF_UnpinBlock(bucketBlock);
    //         BF_Block_Destroy(&bucketBlock);
    //         break;
    //     }

    //     BF_UnpinBlock(bucketBlock);
    //     BF_Block_Destroy(&bucketBlock);
    // }

    // // If the bucket is found, print all entries
    // if (targetBucket != NULL) {
    //     printf("Entries in Bucket %d:\n", targetBucket->bucket_id);

    //     for (int i = 0; i - 2 < targetBucket->bucket_size; i + 2) {
    //         BF_Block *block;
    //         BF_Block_Init(&block);
    //         CALL_BF(BF_GetBlock(openFiles[indexDesc].info->fileDesc, targetBucket->block_id + i, block));
    //         void *blockData = BF_Block_GetData(block);
    //         Record *record = (Record *)((char *)blockData + sizeof(Record) * i);

    //         printf("ID: %d, Name: %s, Surname: %s, City: %s\n", record->id, record->name, record->surname, record->city);

    //         BF_UnpinBlock(block);
    //         BF_Block_Destroy(&block);
    //     }
    // } else {
    //     printf("Bucket for ID %d not found.\n", *id);
    // }

    // // Unpin and destroy blocks
    // BF_Block_SetDirty(infoBlock);
    // BF_UnpinBlock(infoBlock);
    // BF_Block_Destroy(&infoBlock);

    // BF_Block_SetDirty(hashTableBlock);
    // BF_UnpinBlock(hashTableBlock);
    // BF_Block_Destroy(&hashTableBlock);

    // BF_Block_SetDirty(directoryBlock);
    // BF_UnpinBlock(directoryBlock);
    // BF_Block_Destroy(&directoryBlock);

    return HT_OK;

}



// Συναρτήσεις που δημιουργήσαμε:

// HT_ErrorCode HT_Create_File_Array(void* fileArray) {
//   void* fileArray = malloc(20 * sizeof(HT_Info));       // Δυναμική δέσμευση ενός "μπλοκ" μνήμης, μεγέθους 20 φορές το μέγεθος της δομής 'HT_Info',
//                                                         // στο οποίο δείχνει ο δείκτης 'fileArray'.
//   return HT_OK;
// }

HT_ErrorCode HT_Destroy_File_Array(void* fileArray) {
  // void* arrayIndex = fileArray;
  
  // for(int i = 0; i < 20; i++) {
  //   CALL_BF(HT_CloseFile(&arrayIndex));
  //   arrayIndex = fileArray + sizeof(HT_Info);
  // }
  // free(fileArray);
  // return HT_OK;
}


// #define DIRECTORY_SIZE 2
// #define BUCKET_SIZE 2

// Node structure for linked list in each bucket


// Hash function
unsigned int hash(unsigned int key, unsigned int depth) {
  unsigned int hashValue = key * 999999937;
  hashValue = hashValue >> (32 - depth);
  return hashValue;
}

int max_bits(int maxNumber) {
  int maxBits = 0;
  while (maxNumber > 0) {
      maxNumber >>= 1;
      maxBits++;
  }
  return maxBits;
}
void HT_PrintMetadata(void *data) {
  HT_Info *info_ptr = (HT_Info *)data;

  printf("Contents of the first block:\n");
  printf("fileType: %s\n", info_ptr->fileType);
  printf("fileName: %s\n", info_ptr->fileName);
  printf("hash_field: %s\n", info_ptr->hash_field);
  printf("globalDepth: %d\n", info_ptr->globalDepth);
  printf("indexDesc (place in the array): %d\n", info_ptr->indexDesc);
  printf("fileDesc: %d\n", info_ptr->fileDesc);
  printf("rec_num: %d\n", info_ptr->total_num_of_recs);
  printf("num_of_blocks: %d\n", info_ptr->num_of_blocks);
  printf("HT_Info_size: %d\n", info_ptr->HT_Info_size);
  

  // printf("record_size: %d\n", info_ptr->record_size);
  // printf("num_of_buckets: %d\n", info_ptr->num_of_buckets);
}


int power(int num) {
	int two = 1;
  for(int i = 0; i < num; i++)
		two *= 2;
	return two;
}

void HashTable_resize(int** hash_table, HT_Info *info) {
  int newSize = power(info->globalDepth);
  int oldSize = info->num_of_blocks - 1;

  *hash_table = (int *)realloc(*hash_table, newSize * sizeof(int));
  if (*hash_table == NULL) {
    exit(1);
  } else {
    for (; oldSize < newSize; oldSize++) {
      (*hash_table)[oldSize] = -1;
    }
  }
}



void HashTable_deallocate(int* hash_table) {
  // free(hash_table->directory_ids);
  // hash_table->directory_ids = NULL;

  // free(openFiles[nextAvailableIndex-1].hashTable);
}

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
  // Ελέγχουμε αν το αρχείο υπάρχει ήδη.
  BF_ErrorCode code = BF_CreateFile(filename);
  if (code == BF_FILE_ALREADY_EXISTS)
    return HT_ERROR;

  // Ανοίγουμε το αρχείο και επιστρέφουμε το file descriptor του στην μεταβλητή fd.
  int fd;
  CALL_BF(BF_OpenFile(filename, &fd));

  // Ορίζουμε, αρχικοποιούμε και δεσμεύουμε ένα block, για τα μεταδεδομένα του αρχείου.
  BF_Block *infoBlock;
  BF_Block_Init(&infoBlock);
  CALL_BF(BF_AllocateBlock(fd, infoBlock));    
  
  // Αυξάνουμε την μεταβλητή που κρατάει την τιμή του πλήθους των block.
  int num_of_blocks = 1;

  // Ορίζουμε έναν δείκτη που δείχνει στα δεδομένα του block των μεταδεδομένων.
  void *infoData = BF_Block_GetData(infoBlock);

  HT_Info *info = (HT_Info *)infoData;

  strncpy(info->fileType, "Hash File", sizeof(info->fileType) - 1);
  strncpy(info->fileName, filename, sizeof(info->fileName) - 1);
  strncpy(info->hash_field, "id", sizeof(info->hash_field) - 1);

  info->fileDesc = fd;
  info->indexDesc = nextAvailableIndex;
  info->block = infoBlock;
  info->total_num_of_recs = 0;
  info->record_size = sizeof(Record);
  info->num_of_blocks = num_of_blocks;
  info->num_of_buckets = 0;
  info->HT_Info_size = sizeof(HT_Info);

  BF_Block_SetDirty(infoBlock);
  BF_UnpinBlock(infoBlock);
  
  printf("\nContents of the first block:\n");
  printf("fileType: %s\n", info->fileType);
  printf("fileName: %s\n", info->fileName);
  printf("HT_Info_size: %d\n", info->HT_Info_size);
  printf("hash_field: %s\n", info->hash_field);
  printf("indexDesc (place in the array): %d\n", info->indexDesc);
  printf("fileDesc: %d\n", info->fileDesc);
  printf("&block: %p\n", info->block);
  printf("rec_num: %d\n", info->total_num_of_recs);
  printf("record_size: %d\n", info->record_size);
  printf("num_of_blocks: %d\n", info->num_of_blocks);
  printf("num_of_buckets: %d\n\n", info->num_of_buckets);
  

  // Ορίζουμε, αρχικοποιούμε και δεσμεύουμε ένα block, για ον πίνακα κατακερματισμού.
  BF_Block *hashTableBlock;
  BF_Block_Init(&hashTableBlock);
  CALL_BF(BF_AllocateBlock(fd, hashTableBlock));
  
  // Αυξάνουμε την μεταβλητή που κρατάει την τιμή του πλήθους των block.
  num_of_blocks ++;

  // Ορίζουμε έναν δείκτη που δείχνει στα δεδομένα του block των μεταδεδομένων.
  void *hashTableData = BF_Block_GetData(hashTableBlock);

  HashTable *hashTable = (HashTable *)hashTableData;

  hashTable->block_id = num_of_blocks - 1;
  hashTable->global_depth = depth;
  hashTable->num_of_directories = 0;
  hashTable->directory = NULL;
  hashTable->HashTable_size = sizeof(HashTable);
  

  printf("\nContents of the Hash Table:\n");
  printf("block_id: %d\n", hashTable->block_id);
  printf("HashTable_size: %d\n", hashTable->HashTable_size);
  printf("global_depth: %d\n\n", hashTable->global_depth);
  printf("num_of_buckets: %d\n", hashTable->num_of_directories);
  printf("next: %p\n", hashTable->directory);




  
  // Set the pointers in OpenFileInfo to point to the corresponding blocks
  openFiles[nextAvailableIndex].info = info;
  openFiles[nextAvailableIndex].hash_table = hashTable;



  BF_Block_SetDirty(hashTableBlock);
  BF_UnpinBlock(hashTableBlock);
  BF_Block_Destroy(&hashTableBlock);


  info->num_of_blocks = num_of_blocks;

  BF_Block_SetDirty(infoBlock);
  BF_UnpinBlock(infoBlock);
  BF_Block_Destroy(&infoBlock);

  CALL_BF(BF_CloseFile(fd));
  return HT_OK;
}


/* * * * * * * HT_OpenIndex * * * * * * */
HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  // Αρχικοποιούμε τα περιεχόμενα του δείκτη.
  *indexDesc = -1;
  
  for(int i = 0; i < MAX_OPEN_FILES; i++) {
    // Συγκρίνουμε το όνομα που μας δόθηκε με τα ονόματα των ανοικτών αρχείων στον πίνακα.
    if(strcmp(openFiles[i].info->fileName, fileName) == 0) {
      *indexDesc = i;   // Καταχωρούμε το i στο indexDesc.
      break;
    }
  }
  
  if(*indexDesc != -1) {
    BF_OpenFile(fileName, &(openFiles[*indexDesc].info->fileDesc));
  return HT_OK;
  }

  return HT_ERROR;
}


HT_ErrorCode HT_CloseFile(int indexDesc) {
  // Check if the provided index is within valid bounds
 if (indexDesc < 0 || indexDesc >= MAX_OPEN_FILES || openFiles[indexDesc].info->fileDesc == -1) {
    // Handle error: invalid index
    return HT_ERROR;  // You should define appropriate error codes
  }

  // Get the file descriptor from the open files array
  int fileDesc = openFiles[indexDesc].info->fileDesc;

  HashTable_deallocate(openFiles[indexDesc].hash_table);
  BF_ErrorCode code = BF_CloseFile(fileDesc);


  // Close the file
  if (code != BF_OK) {
      // Handle file close error
      return HT_ERROR;  // You should define appropriate error codes
  }

  return HT_OK;
}


/* * * * * * * * * HT_InsertEntry * * * * * * * * */
HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  // Φορτώνουμε το block με τα μεταδεδομένα του αρχείου στην buffer.
  BF_Block* infoBlock;
  BF_Block_Init(&infoBlock);
  CALL_BF(BF_GetBlock(openFiles[indexDesc].info->fileDesc, 0, infoBlock));
  HT_Info* info = (HT_Info *) BF_Block_GetData(infoBlock);
  
  // Φορτώνουμε το block με τα μεταδεδομένα του πίνακα κατακερματισμού στην buffer.
  BF_Block* hashTableBlock;
  BF_Block_Init(&hashTableBlock);
  CALL_BF(BF_GetBlock(info->fileDesc, 1, hashTableBlock));
  HashTable* hashTable = (HashTable *) BF_Block_GetData(hashTableBlock);
  
  // Υπολογίζουμε την τιμή κατακερματισμού του εκάστοτε record.id.
  int hashValue = hash(record.id, hashTable->global_depth);
  

  void* directoryData;
  void* bucketData;

  BF_Block* directoryBlock;
  Directory* directory;
  // Στην 1η κλήση της συνάρτησης δημιουργούμε τον αναγκαίο αριθμό από directories.
  for(int i = 0; hashTable->num_of_directories < power(hashTable->global_depth); i++) {
    if(i == 0)
      HashTable_resize(hashTable);

    // Αρχικοποιούμε και δεσμεύουμε ένα το μπλοκ.
    BF_Block_Init(&directoryBlock);
    CALL_BF(BF_AllocateBlock(info->fileDesc, directoryBlock));

    // Ο δείκτης δείχνει στα δεδομένα του μπλοκ που μόλις δεσμεύσαμε.
    directory = (Directory *)BF_Block_GetData(directoryBlock);

    // Αποθηκεύουμε τον συνολικό αριθμό μπλοκ στην δομή. 
    BF_GetBlockCounter(info->fileDesc, &info->num_of_blocks);

    // Αρχικοποιήση
    directory->directory_id = info->num_of_blocks-1;
    hashTable->directory_ids[i] = directory->directory_id;

    directory->bucket_id = -1;
    directory->bucket_size = 0;
    directory->local_depth = hashTable->global_depth;
    directory->hash_value = i;
    directory->buddies = 0;

    // Δεν δεσμεύουμε bucket στο directory, αφού μπορεί να παραμείνει άδειος.
    directory->bucket_num = 0;
    directory->bucket = NULL;
    directory->directory_size_bytes = sizeof(Directory);

    hashTable->num_of_directories ++;

    // Κάνουμε το μπλοκ βρώμικο, το ξεκαρφιτσώνουμε και το καταστρέφουμε.
    BF_Block_SetDirty(directoryBlock);
    CALL_BF(BF_UnpinBlock(directoryBlock));
    BF_Block_Destroy(&directoryBlock);
    
    BF_Block_SetDirty(hashTableBlock);
   }

  // Φέρνουμε ένα-ένα τα directories στην μνήμη έως ότου να βρούμε αυτό με το σωστό hash value.
  int directory_id = -1;
  for(int i = 0; i < hashTable->num_of_directories; i++) {
    // Κάθε φορά αρχικοποιούμε το block μέσα στον βρόχο. (*)
    BF_Block_Init(&directoryBlock);
    CALL_BF(BF_GetBlock(info->fileDesc, hashTable->directory_ids[i], directoryBlock));
    directory = (Directory *)BF_Block_GetData(directoryBlock);

    // Ελέγχουμε αν το hash value της δοθείσας εγγραφής ταυτίζεται με το hash value κάποιου
    // ήδη υπάρχοντος directory. Αν ναι, τότε κρατάμε το directory_id του σε μια προσωρινή μεταβλητή.
    if(directory->hash_value == hashValue) {
      directory_id = directory->directory_id;
      break;
    }
    else {
      // (*) Και κάθε φορά το ξεκαρφιτσώνουμε και το καταστρέφουμε.
      CALL_BF(BF_UnpinBlock(directoryBlock));
      BF_Block_Destroy(&directoryBlock);
      }
  }

  BF_Block *bucket;

  if(directory_id != -1) {
    BF_Block_Init(&directory->bucket);
    
    if(directory->bucket_num == 0 && directory->buddies == 0) {
      BF_AllocateBlock(info->fileDesc, directory->bucket);
      info->num_of_blocks ++;
      info->num_of_buckets ++;
      directory->bucket_num = 1;
      directory->bucket_id = info->num_of_blocks - 1;

    }
    else {
      BF_GetBlock(info->fileDesc, directory->bucket_id, directory->bucket);
    }

    BF_Block *bucketData = (BF_Block *)BF_Block_GetData(directory->bucket);

    Record *recordinBlock = (Record *)((char *)bucketData + sizeof(Record) * directory->bucket_size);
    
    // Ελέγχουμε αν χωράει η εγγραφή στο block
    if (sizeof(Record) * (directory->bucket_size + 1) < BF_BLOCK_SIZE) {
      // Copy the record into the block
      memcpy(recordinBlock, &record, sizeof(Record));
      
      printf("hash: %d, ID: %d, name: %s, surname: %s, city: %s\n", hashValue, recordinBlock->id, recordinBlock->name, recordinBlock->surname, recordinBlock->city);
      directory->bucket_size++;
      printf("Eggrafes mexri stigmhs sto bucket %d: %d\n\n", directory->hash_value, directory->bucket_size);

      info->total_num_of_recs ++;


      // Κάνουμε τα μπλοκς βρώμικα, τα ξεκαρφιτσώνουμε και τα καταστρέφουμε.

      BF_Block_SetDirty(directory->bucket);
      CALL_BF(BF_UnpinBlock(directory->bucket));
      BF_Block_Destroy(&directory->bucket);

      BF_Block_SetDirty(directoryBlock);
      CALL_BF(BF_UnpinBlock(directoryBlock));
      BF_Block_Destroy(&directoryBlock);


      CALL_BF(BF_UnpinBlock(hashTableBlock));
      BF_Block_Destroy(&hashTableBlock);

      BF_Block_SetDirty(infoBlock);
      CALL_BF(BF_UnpinBlock(infoBlock));
      BF_Block_Destroy(&infoBlock);

      return HT_OK;
    }
    else if(directory->local_depth == hashTable->global_depth){ // Σε αυτήν την περίπτωση η εγγραφή δεν χωράει στο block.
      // (1) Διπλασιάζουμε τον πίνακα κατακερματισμού.
      printf("Den xwraei\n");
      
      printf("Eggrafh pros eisagwgh:\nhash: %d, ID: %d, name: %s, surname: %s, city: %s\n\nPalies eggrafes:\n", hashValue, record.id, record.name, record.surname, record.city);
      
      
      // Δημιουργούμε ένα νέο directory για τις εγγραφές που δεν χωρούσαν.
      BF_Block *new_directory_block;
      BF_Block_Init(&new_directory_block);
      CALL_BF(BF_AllocateBlock(info->fileDesc, new_directory_block));
      Directory *newDirectory = (Directory *)BF_Block_GetData(new_directory_block);

      // hashTable->num_of_directories ++;

      // (3) Αύξηση του global depth κατά 1.
      hashTable->global_depth ++;

      info->num_of_blocks ++;
      info->num_of_buckets ++;

      newDirectory->directory_id = info->num_of_blocks-1;
      HashTable_resize(hashTable);
      
      for(int i = 0; i < power(hashTable->global_depth); i++)
        if(hashTable->directory_ids[i] == -1) {
          hashTable->directory_ids[i] = newDirectory->directory_id;
          break;
        }
      
      hashTable->num_of_directories ++;
      
      newDirectory->bucket_num = 0;
      newDirectory->bucket_size = 0;
      newDirectory->buddies = 0;
      newDirectory->directory_size_bytes = sizeof(Directory);
      
      // Η καινούρια τιμή κατακερματισμού για τις υπάρχουσες θέσεις στον πίνακα προκύπτει
      // από την δεξιά ολίσθηση της παλιάς τιμής κατά 1.
      directory->hash_value <<= 1;
      newDirectory->hash_value = directory->hash_value + 1;
    

      // Ενημέρωση του ολικού βάθους.
      newDirectory->local_depth = hashTable->global_depth;
      directory->local_depth = hashTable->global_depth;


      // (4) Διάσπαση του κάδου σε 2.
      BF_Block_Init(&newDirectory->bucket);
      CALL_BF(BF_AllocateBlock(info->fileDesc, newDirectory->bucket));


      newDirectory->bucket_id = newDirectory->directory_id + 1;
      info->num_of_blocks ++;

      // (6) Επανυπολογισμός της τιμής κατακερματισού όλων των εγγραφών στον ήδη υπάρχων κάδο.
      void* newBucketdata = BF_Block_GetData(newDirectory->bucket);
      void* data;
      Directory *currentDirectory;

      Record *records;
      int temp = directory->bucket_num + 1;
      int oldDirBucketSize = directory->bucket_size;
      directory->bucket_size = 0;

      for(int i = 0; i < temp; i++) {
        if(oldDirBucketSize > 0) {
          // Επανατοποθετούμε μία-μία τις εγγραφές στον κατάλληλο κάδο.
          records = (Record *)((char *)bucketData + sizeof(Record) * i);
          hashValue = hash(records->id, directory->local_depth);
          oldDirBucketSize --;
        }
        else {
          // Μόλις τελειώσουμε με τις ήδη υπάρχουσες εγγραφές,
          // εξετάζουμε την τιμή κατακερματισμού της δοθείσας.
          hashValue = hash(record.id, directory->local_depth);
          records = &record;
          info->total_num_of_recs ++;
        }

        // Αναλόγως το hash value της εγγραφής οι δείκτες data και currentDirectory δείχνουν
        // στα δεδομένα του bucket και του directory στα οποία θα καταλήξει η εγγραφή.
        if(hashValue == directory->hash_value && directory->bucket_size * sizeof(Record) < BF_BLOCK_SIZE) {
          data = bucketData;
          currentDirectory = directory;
        }
        else {
          printf("skata\n");
          // CALL_BF(HT_InsertEntry(indexDesc, *records));
        }
        if(hashValue == newDirectory->hash_value && newDirectory->bucket_size * sizeof(Record) < BF_BLOCK_SIZE){ 
          data = newBucketdata;
          currentDirectory = newDirectory;
        }
        Record *currentRecord = (Record *)((char *)data + sizeof(Record) * currentDirectory->bucket_size);
        memcpy(currentRecord, records, sizeof(Record));
        printf("hash: %d, ID: %d, name: %s, surname: %s, city: %s\n", hashValue, records->id, records->name, records->surname, records->city);
        printf("Eggrafes mexri stigmhs sto bucket me id %d: %d\n\n", currentDirectory->directory_id, currentDirectory->bucket_size+1);

        currentDirectory->bucket_size ++;
      }
      
      BF_Block_SetDirty(newDirectory->bucket);
      CALL_BF(BF_UnpinBlock(newDirectory->bucket));
      BF_Block_Destroy(&newDirectory->bucket);

      BF_Block_SetDirty(new_directory_block);
      CALL_BF(BF_UnpinBlock(new_directory_block));
      BF_Block_Destroy(&new_directory_block);


      
      HashTable_resize(hashTable);
      
      // Δημιουργούμε τις υπόλοιπες θέσεις (directories) τον πίνακα κατακερματισμού.
      for(; hashTable->num_of_directories < power(hashTable->global_depth); ) {
        BF_Block_Init(&new_directory_block);
        CALL_BF(BF_AllocateBlock(info->fileDesc, new_directory_block));
        newDirectory = (Directory *)BF_Block_GetData(new_directory_block);

        info->num_of_blocks ++;
        info->num_of_buckets ++;
        newDirectory->directory_id = info->num_of_blocks-1;

        for(int i = 0; i < power(hashTable->global_depth); i++)
          if(hashTable->directory_ids[i] == -1) {
            hashTable->directory_ids[i] = newDirectory->directory_id;
            break;
          }

        
        newDirectory->bucket_num = 0;
        newDirectory->bucket_size = 0;
        newDirectory->local_depth = hashTable->global_depth;    
        newDirectory->directory_size_bytes = sizeof(Directory);
        printf("newDirectory->directory_size_bytes: %d \n\n", newDirectory->directory_size_bytes);

        for(int k = 0; k < power(hashTable->global_depth) - hashTable->num_of_directories / 2; k ++) {
          BF_Block* oldDirectoryBlock;
          BF_Block_Init(&oldDirectoryBlock);
          CALL_BF(BF_GetBlock(info->fileDesc, directory_id + k, oldDirectoryBlock));
          Directory *oldDirectory = (Directory *)BF_Block_GetData(oldDirectoryBlock);


          if(oldDirectory->bucket_num == 1 && oldDirectory->buddies == 0 && oldDirectory->local_depth < hashTable->global_depth) {
            oldDirectory->hash_value <<= 1;
            newDirectory->hash_value = oldDirectory->hash_value + 1;
            newDirectory->buddies = 1;
            newDirectory->bucket_id = oldDirectory->bucket_id;
            newDirectory->bucket = oldDirectory->bucket;

            
    

            BF_Block_SetDirty(oldDirectoryBlock);
            CALL_BF(BF_UnpinBlock(oldDirectoryBlock));
            BF_Block_Destroy(&oldDirectoryBlock);
            break;
          }

            hashTable->num_of_directories ++;
        }
        
        BF_Block_SetDirty(new_directory_block);
        CALL_BF(BF_UnpinBlock(new_directory_block));
        BF_Block_Destroy(&new_directory_block);
      }

      BF_Block_SetDirty(directoryBlock);
      CALL_BF(BF_UnpinBlock(directoryBlock));
      BF_Block_Destroy(&directoryBlock);
      ////
      ////
      ////
      ////
      ////
    }
    else {
      printf("\n\n2h periptwsh\n\n\n");
      // (4) Διάσπαση του κάδου σε 2.
      BF_Block *buddyBlock;
      BF_Block_Init(&buddyBlock);
      CALL_BF(BF_GetBlock(info->fileDesc, directory->bucket_id, buddyBlock));
    
    
    }
    BF_Block_SetDirty(hashTableBlock);
    CALL_BF(BF_UnpinBlock(hashTableBlock));
    BF_Block_Destroy(&hashTableBlock);

    BF_Block_SetDirty(infoBlock);
    CALL_BF(BF_UnpinBlock(infoBlock));
    BF_Block_Destroy(&infoBlock);
  }
  else {
    printf("AKYRO\n\n");
    CALL_BF(BF_UnpinBlock(directoryBlock));
    BF_Block_Destroy(&directoryBlock);

    CALL_BF(BF_UnpinBlock(hashTableBlock));
    BF_Block_Destroy(&hashTableBlock);

    CALL_BF(BF_UnpinBlock(infoBlock));
    BF_Block_Destroy(&infoBlock);
    return HT_ERROR;
  }

  
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
  
  printf("indexDesc (place in the array): %d\n", info_ptr->indexDesc);
  printf("fileDesc: %d\n", info_ptr->fileDesc);
  printf("block: %p\n", info_ptr->block);
  printf("rec_num: %d\n", info_ptr->total_num_of_recs);
  printf("record_size: %d\n", info_ptr->record_size);
  printf("num_of_blocks: %d\n", info_ptr->num_of_blocks);
  printf("num_of_buckets: %d\n", info_ptr->num_of_buckets);
}


int power(int num) {
	int two = 1;
  for(int i = 0; i < num; i++)
		two *= 2;
	return two;
}

void HashTable_resize(HashTable* hash_table) {
  int oldSize = hash_table->num_of_directories, newSize = power(hash_table->global_depth);
  if(hash_table->num_of_directories < newSize) {
    int* new_block = (int*)realloc(hash_table->directory_ids, newSize * sizeof(int));
    if (new_block != NULL) {
        // If the size has increased, copy values from the old block to the new block
        for (int i = 0; i < newSize; i++) {
          if(i < oldSize)
            new_block[i] = hash_table->directory_ids[i];
          else
            new_block[i] = -1;        // Αρχικοποίηση με -1 για να ξέρουμε πότε η θέση δεν περιέχει ακόμα κάποια διεύθυνση.
        }

        hash_table->directory_ids = new_block;
        hash_table->HashTable_size = sizeof(HashTable);
        printf("hash_table->HashTable_size: %d\n\n", hash_table->HashTable_size);
        if(hash_table->HashTable_size >= BF_BLOCK_SIZE)
          printf("malakia\n\n");
    }
    else {
        exit(1);
    }
  }
}

void HashTable_deallocate(HashTable* hash_table) {
  free(hash_table->directory_ids);
  hash_table->directory_ids = NULL;
}
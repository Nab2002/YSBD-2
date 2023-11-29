#ifndef HASH_FILE_H
#define HASH_FILE_H

typedef enum HT_ErrorCode {
  HT_OK,
  HT_ERROR
} HT_ErrorCode;

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

// Δομή που αποθηκεύει ορισμένες πληροφορίες σχετικά με ένα αρχείο (κατακερματισμού).
typedef struct HT_Info {
	char fileType [20];				// eg. 'hash' file
	char fileName[20];
	int indexDesc;					// θέση στον πίνακα
	int fileDesc;
	char hash_field[10];			// Το κλειδί του πίνακα κατακερματισμού (eg. id).
	BF_Block *next;
	int total_num_of_recs;			// συνολικός αριθμός εγγραφών σε όλο το αρχείο
	int num_of_blocks;				// συνολικός αριθμός μπλοκ στο αρχείο
	int HT_Info_size;				// sizeof(HT_Info)
} HT_Info;


typedef struct Block_Info {
	int block_id;				// αναγνωριστικός αριθμός του μπλοκ
	int directories_hash[8];	// το μπλοκ χωράει έως 8 directories
	int num_of_directories;		// αριθμός των directories που περιέχει.
	BF_Block* next;				// δείκτης στο επόμενο μπλοκ του αρχείου.
} Block_Info;

typedef struct Directory {
	int directory_id;			// αναγνωριστικός αριθμός του Directory
	int bucket_owner_id;		// αναγνωριστικός αριθμός του directory που έχει το bucket
	int bucket_id;			// αναγνωριστικός αριθμός του bucket
    int local_depth;		// συνολικός αριθμός εγγραφών στο bucket
	int bucket_size;
    BF_Block *bucket;
	int bucket_num;			// συνολικός αριθμός buckets στο Directory
	int hash_value;			// αναγνωριστικός αριθμός bucket
	int directory_size_bytes;	// sizeof(Bucket)
	int buddies;				// 1: yes, 0: no
} Directory;


// Hash table structure
typedef struct HashTable {
    int block_id;				// αναγνωριστικός αριθμός του μπλοκ
	int global_depth;      		// Global depth of the directory
    int num_of_directories;		// συνολικός αριθμός buckets στο ευρετήριο
	Directory* directory;
	int* directory_ids;			// αναγνωριστικός αριθμός όλων των directories
	int HashTable_size;			// sizeof(HashTable)
} HashTable;


typedef struct {
	int place;				// θέση στον πίνακα
	BF_Block *firstBlock;	// δείκτης στο 1ο μπλοκ του αρχείου
	char filename [20];
} OpenFileInfo;



/*
* Η συνάρτηση HT_Init χρησιμοποιείται για την αρχικοποίηση κάποιων δομών που μπορεί να χρειαστείτε. 
* Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
*/
HT_ErrorCode HT_Init();

/*
 * Η συνάρτηση HT_CreateIndex χρησιμοποιείται για τη δημιουργία και κατάλληλη αρχικοποίηση ενός άδειου αρχείου κατακερματισμού με όνομα fileName. 
 * Στην περίπτωση που το αρχείο υπάρχει ήδη, τότε επιστρέφεται ένας κωδικός λάθους. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HΤ_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CreateIndex(
	const char *fileName,		/* όνομα αρχείου */
	int depth
	);


/*
 * Η ρουτίνα αυτή ανοίγει το αρχείο με όνομα fileName. 
 * Εάν το αρχείο ανοιχτεί κανονικά, η ρουτίνα επιστρέφει HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_OpenIndex(
	const char *fileName, 		/* όνομα αρχείου */
  int *indexDesc            /* θέση στον πίνακα με τα ανοιχτά αρχεία  που επιστρέφεται */
	);

/*
 * Η ρουτίνα αυτή κλείνει το αρχείο του οποίου οι πληροφορίες βρίσκονται στην θέση indexDesc του πίνακα ανοιχτών αρχείων.
 * Επίσης σβήνει την καταχώρηση που αντιστοιχεί στο αρχείο αυτό στον πίνακα ανοιχτών αρχείων. 
 * Η συνάρτηση επιστρέφει ΗΤ_OK εάν το αρχείο κλείσει επιτυχώς, ενώ σε διαφορετική σε περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CloseFile(
	int indexDesc 		/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	);

/*
 * Η συνάρτηση HT_InsertEntry χρησιμοποιείται για την εισαγωγή μίας εγγραφής στο αρχείο κατακερματισμού. 
 * Οι πληροφορίες που αφορούν το αρχείο βρίσκονται στον πίνακα ανοιχτών αρχείων, ενώ η εγγραφή προς εισαγωγή προσδιορίζεται από τη δομή record. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_InsertEntry(
	int indexDesc,	/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	Record record		/* δομή που προσδιορίζει την εγγραφή */
	);

/*
 * Η συνάρτηση HΤ_PrintAllEntries χρησιμοποιείται για την εκτύπωση όλων των εγγραφών που το record.id έχει τιμή id. 
 * Αν το id είναι NULL τότε θα εκτυπώνει όλες τις εγγραφές του αρχείου κατακερματισμού. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HP_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_PrintAllEntries(
	int indexDesc,	/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	int *id 				/* τιμή του πεδίου κλειδιού προς αναζήτηση */
	);



// Συναρτήσεις που δημιουργήσαμε:

/*
 * Η συνάρτηση HT_Create_File_Array χρησιμοποιείται για την δημιουργία ένος πίνακα (στην μνήμη) ο οποίος κρατάει 
 * όλα τα ανοιχτά αρχεία, αποδίδοντας τους μια τιμή int* indexDesc που αντιστοιχεί στην θέση στην οποία βρισκεται
 * το αρχείο που μόλις ανοίχτηκε. 
 */
HT_ErrorCode HT_Create_File_Array (
	void* fileArray
	);


HT_ErrorCode HT_Destroy_File_Array(
	void* fileArray
	);

void HT_PrintMetadata(void *data);


/*
 * Η συνάρτηση power επιτρέφει το 2^k, όπυ k = num;
 */
int power(int num);

unsigned int hash(unsigned int key, unsigned int depth);

int max_bits(int maxNumber);

void HashTable_resize(HashTable* hash_table);

void HashTable_deallocate(HashTable* hash_table);

#endif // HASH_FILE_H
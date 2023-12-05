#ifndef HASH_FILE_H
#define HASH_FILE_H
#define TABLE_SIZE 3

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
	char fileName [20];
	char hash_field [20];			// Το κλειδί του πίνακα κατακερματισμού (eg. id).
	int total_num_of_recs;			// συνολικός αριθμός εγγραφών σε όλο το αρχείο
	int globalDepth;
	int num_of_blocks;				// συνολικός αριθμός μπλοκ στο Hash Table
	
} HT_Info;


typedef struct Block_Info {
	int block_id;						// αναγνωριστικός αριθμός του μπλοκ
	int bucket_size;					// αριθμός των records που περιέχει
	int local_depth;					// τοπικό βάθος του bucket
	// int hashvalue;						// temp variable for debugging
	int buddies;						// πλήθος buddies
} Block_Info;




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

HT_ErrorCode HashStatistics(char* filename);

// Συναρτήσεις που δημιουργήσαμε:

/*
 * Η συνάρτηση HT_Create_File_Array χρησιμοποιείται για την δημιουργία ένος πίνακα (στην μνήμη) ο οποίος κρατάει 
 * όλα τα ανοιχτά αρχεία, αποδίδοντας τους μια τιμή int* indexDesc που αντιστοιχεί στην θέση στην οποία βρισκεται
 * το αρχείο που μόλις ανοίχτηκε. 
 */

void HT_PrintMetadata(void *data);

void Print_Hash_Table(int *hash_table, HT_Info *info);


/*
 * Η συνάρτηση power επιτρέφει το 2^k, όπυ k = num;
 */
int power(int num);

unsigned int hash(unsigned int key, unsigned int depth);

int max_bits(int maxNumber);

void HashTable_resize(int** hash_table, HT_Info *info);

#endif // HASH_FILE_H
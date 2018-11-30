#include "globals.h"

/*
 Student Name: Baran Usluel
 Date: 11/7/2018

=======================
ECE 2035 Project 2-1:
=======================
This file provides definition for the structs and functions declared in the
header file. It also contains helper functions that are not accessible from
outside of the file.

FOR FULL CREDIT, BE SURE TO TRY MULTIPLE TEST CASES and DOCUMENT YOUR CODE.

===================================
Naming conventions in this file:
===================================
1. All struct names use camel case where the first letter is capitalized.
  e.g. "HashTable", or "HashTableEntry"

2. Variable names with a preceding underscore "_" will not be called directly.
  e.g. "_HashTable", "_HashTableEntry"

  Recall that in C, we have to type "struct" together with the name of the struct
  in order to initialize a new variable. To avoid this, in hash_table.h
  we use typedef to provide new "nicknames" for "struct _HashTable" and
  "struct _HashTableEntry". As a result, we can create new struct variables
  by just using:
    - "HashTable myNewTable;"
     or
    - "HashTableEntry myNewHashTableEntry;"

  The preceding underscore "_" simply provides a distinction between the names
  of the actual struct defition and the "nicknames" that we use to initialize
  new structs.
  [See Hidden Definitions section for more information.]

3. Functions, their local variables and arguments are named with camel case, where
  the first letter is lower-case.
  e.g. "createHashTable" is a function. One of its arguments is "numBuckets".
       It also has a local variable called "newTable".

4. The name of a struct member is divided by using underscores "_". This serves
  as a distinction between function local variables and struct members.
  e.g. "num_buckets" is a member of "HashTable".

*/

/****************************************************************************
* Include the Public Interface
*
* By including the public interface at the top of the file, the compiler can
* enforce that the function declarations in the the header are not in
* conflict with the definitions in the file. This is not a guarantee of
* correctness, but it is better than nothing!
***************************************************************************/
#include "hash_table.h"


/****************************************************************************
* Include other private dependencies
*
* These other modules are used in the implementation of the hash table module,
* but are not required by users of the hash table.
***************************************************************************/
#include <stdlib.h>   // For malloc and free
#include <stdio.h>    // For printf


/****************************************************************************
* Hidden Definitions
*
* These definitions are not available outside of this file. However, because
* the are forward declared in hash_table.h, the type names are
* available everywhere and user code can hold pointers to these structs.
***************************************************************************/
/**
 * This structure represents an a hash table.
 * Use "HashTable" instead when you are creating a new variable. [See top comments]
 */
struct _HashTable {
    /** The array of pointers to the head of a singly linked list, whose nodes
        are HashTableEntry objects */
    HashTableEntry** buckets;

    /** The hash function pointer */
    HashFunction hash;

    /** The number of buckets in the hash table */
    unsigned int num_buckets;
};

/**
 * This structure represents a hash table entry.
 * Use "HashTableEntry" instead when you are creating a new variable. [See top comments]
 */
struct _HashTableEntry {
    /** The key for the hash table entry */
    unsigned int key;

    /** The value associated with this hash table entry */
    void* value;

    /**
    * A pointer pointing to the next hash table entry
    * NULL means there is no next entry (i.e. this is the tail)
    */
    HashTableEntry* next;
};


/****************************************************************************
* Private Functions
*
* These functions are not available outside of this file, since they are not
* declared in hash_table.h.
***************************************************************************/
/**
* createHashTableEntry
*
* Helper function that creates a hash table entry by allocating memory for it on
* the heap. It initializes the entry with key and value, initialize pointer to
* the next entry as NULL, and return the pointer to this hash table entry.
*
* @param key The key corresponds to the hash table entry
* @param value The value stored in the hash table entry
* @return The pointer to the hash table entry
*/
static HashTableEntry* createHashTableEntry(unsigned int key, void* value) {
    // Allocate memory for the new HashTableEntry struct on heap
    HashTableEntry* newEntry = (HashTableEntry*)malloc(sizeof(HashTableEntry));
    if (!newEntry)
        pc.printf("OUT OF MEMORY");
    
    // Initialize struct members
    newEntry->next = NULL;
    newEntry->key = key;
    newEntry->value = value;

    return newEntry;
}

/**
* findItem
*
* Helper function that checks whether there exists the hash table entry that
* contains a specific key.
*
* @param hashTable The pointer to the hash table.
* @param key The key corresponds to the hash table entry
* @return The pointer to the hash table entry, or NULL if key does not exist
*/
static HashTableEntry* findItem(HashTable* hashTable, unsigned int key) {
    // Get hash for the key
    unsigned int hash = hashTable->hash(key);
    // Get pointer to the head
    HashTableEntry* curr = hashTable->buckets[hash];
    // While pointer is not NULL
    while (curr) {
        // If the key is found
        if (curr->key == key) {
            return curr;
        }
        // Move to next entry
        curr = curr->next;
    }
    return NULL;
}

/**
* findPrevious
*
* Helper function that returns the HashTableEntry directly before the
* HashTableEntry with a specific key.
*
* This function will return NULL if a HashTableEntry with the given key
* was not found, or if it was at the head (because there is no previous node).
*
* @param hashTable The pointer to the hash table.
* @param key The key corresponds to the hash table entry
* @return The pointer to the previous entry, or NULL if key does not exist
*/
static HashTableEntry* findPrevious(HashTable* hashTable, unsigned int key) {
    // Get hash for the key
    unsigned int hash = hashTable->hash(key);
    // Get pointer to the head
    HashTableEntry* curr = hashTable->buckets[hash];
    // While current and next entries are not NULL
    while (curr && curr->next) {
        // If next entry has desired key
        if (curr->next->key == key) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/****************************************************************************
* Public Interface Functions
*
* These functions implement the public interface as specified in the header
* file, and make use of the private functions and hidden definitions in the
* above sections.
****************************************************************************/
// The createHashTable is provided for you as a starting point.
HashTable* createHashTable(HashFunction hashFunction, unsigned int numBuckets) {
    // The hash table has to contain at least one bucket. Exit gracefully if
    // this condition is not met.
    if (numBuckets==0) {
        printf("Hash table has to contain at least 1 bucket...\n");
        exit(1);
    }

    // Allocate memory for the new HashTable struct on heap.
    HashTable* newTable = (HashTable*)malloc(sizeof(HashTable));
    if (!newTable)
        pc.printf("OUT OF MEMORY");

    // Initialize the components of the new HashTable struct.
    newTable->hash = hashFunction;
    newTable->num_buckets = numBuckets;
    newTable->buckets = (HashTableEntry**)malloc(numBuckets*sizeof(HashTableEntry*));
    if (!newTable->buckets)
        pc.printf("OUT OF MEMORY");

    // As the new buckets contain indeterminant values, init each bucket as NULL.
    unsigned int i;
    for (i=0; i<numBuckets; ++i) {
        newTable->buckets[i] = NULL;
    }

    // Return the new HashTable struct.
    return newTable;
}

void destroyHashTable(HashTable* hashTable) {
    unsigned int i;
    HashTableEntry* curr;
    HashTableEntry* next;
    // For every bucket
    for (i = 0; i < hashTable->num_buckets; i++) {
        // Get head pointer in bucket
        curr = hashTable->buckets[i];
        // While the HashTableEntry pointer is not NULL
        while (curr) {
            // Free the value from the heap
            free(curr->value);
            // Get pointer to next entry
            next = curr->next;
            // Free the entry from the heap
            free(curr);
            // Move current pointer to next entry
            curr = next;
        }
    }
    // Free the HashTableEntry** buckets array from the heap
    free(hashTable->buckets);
    // Free the HashTable from the heap
    free(hashTable);
}

void* insertItem(HashTable* hashTable, unsigned int key, void* value) {
    // Find the HashTableEntry if it already exists in HashTable
    HashTableEntry* entry = findItem(hashTable, key);
    // HashTableEntry already exists. Replace value and return old value
    if (entry) {
        void* oldValue = entry->value;
        entry->value = value;
        return oldValue;
    }

    // Create new HashTableEntry from key and value
    entry = createHashTableEntry(key, value);
    // Get hash for the key
    unsigned int hash = hashTable->hash(key);
    // Set next pointer of new entry to old head
    entry->next = hashTable->buckets[hash];
    // Set head to new entry
    hashTable->buckets[hash] = entry;

    return NULL;
}

void* getItem(HashTable* hashTable, unsigned int key) {
    // Find the HashTableEntry if it exists in HashTable
    HashTableEntry* entry = findItem(hashTable, key);
    // HashTableEntry was not found
    if (!entry) {
        return NULL;
    }

    // Return value in the entry
    return entry->value;
}

void* removeItem(HashTable* hashTable, unsigned int key) {
    // Get hash for the key
    unsigned int hash = hashTable->hash(key);
    // Get head entry at the relevant bucket
    HashTableEntry* entry = hashTable->buckets[hash];

    // The key is not present
    if (!entry) {
        return NULL;
    }

    // If the entry with the desired key is at the head
    if (entry->key == key) {
        // Set head to next entry
        hashTable->buckets[hash] = entry->next;
    } else {
        // Find the previous entry
        HashTableEntry* prevEntry = findPrevious(hashTable, key);
        // The key is not present
        if (!prevEntry) {
            return NULL;
        }

        // Get entry that has matching key
        entry = prevEntry->next;
        // Remove the entry from the LinkedList
        prevEntry->next = entry->next;
    }

    // Get value stored in the entry
    void* value = entry->value;
    // Free HashTableEntry from heap
    free(entry);
    return value;
}

void deleteItem(HashTable* hashTable, unsigned int key) {
    // Get hash for the key
    unsigned int hash = hashTable->hash(key);
    // Get head entry at the relevant bucket
    HashTableEntry* entry = hashTable->buckets[hash];

    // The key is not present
    if (!entry) {
        return;
    }

    // If the entry with the desired key is at the head
    if (entry->key == key) {
        // Set head to next entry
        hashTable->buckets[hash] = entry->next;
    } else {
        // Find the previous entry
        HashTableEntry* prevEntry = findPrevious(hashTable, key);
        // The key is not present
        if (!prevEntry) {
            return;
        }

        // Get entry that has matching key
        entry = prevEntry->next;
        // Remove the entry from the LinkedList
        prevEntry->next = entry->next;
    }

    // Free value from heap
    free(entry->value);
    // Free HashTableEntry from heap
    free(entry);
}

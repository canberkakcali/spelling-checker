#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef __linux
    #define stricmp strcasecmp
#endif


// #define ACCESSCOUNT_DEBUG // Prints the hit counts of the words. The more hits a word has, the earlier it will be checked.
// How much to expand the arraylist when the allocated size becomes full
#define SIZE_MULTIPLIER 2
// Average length of the words. Will be used to determine initial buffer size.
#define AVG_WORD_LEN 6

// ARRAYLIST PROTOTYPES
typedef struct {
    char** words;
    int* accessCount; // hit counts of each word
    int wordCount; // total number of words in the list
    int size; // total size available (allocated but not used)
} arrayList;

arrayList* arrayListCreate(int);
void arrayListAdd(arrayList*, char*);
void arrayListPrintHorizontal(arrayList*);
int partition(arrayList*, int, int);
void arrayListQuickSort(arrayList*, int, int);
bool arrayListBinarySearch(arrayList*, int, int, char*);
void arrayListPrintMostAccessed(arrayList*);
void arrayListTouchWord(arrayList*, int);
void arrayListSwapElements(arrayList*, int, int);

// PROGRAM PROTOTYPES
void openFiles(FILE**, FILE**, const char*, const char*);
char* readWordFromFile(FILE*);
void fillDictionary(arrayList***, FILE*);
int indexOfChar(char);
void printDictionary(arrayList**);
void quickSortDictionary(arrayList**);
void checkTextFile(FILE*, arrayList**);
bool checkWord(char*, arrayList**);

/*
*   __________________
*   PROGRAM FUNCTIONS
*   __________________
*
*/

int main(int argc, char** argv) {

    if(argc < 3) {
        printf("Not enough argument.\n"
               "Usage: %s <dictionary file> <file to be checked>\n", argv[0]);
        return 0;
    }

    FILE* fpDictionary; 
    FILE* fpText;
    
    openFiles(&fpDictionary, &fpText, argv[1], argv[2]);

    arrayList** dictionary = (arrayList**) malloc( sizeof(arrayList*) * 26 );
    if(dictionary == NULL) {
        printf("Memory allocation error while creating dictionary.\n");
        return 0;
    }

    fillDictionary(&dictionary, fpDictionary);

    printf("\nDICTIONARY BEFORE SORTING:\n");
    printDictionary(dictionary);

    quickSortDictionary(dictionary);

    printf("\nDICTIONARY AFTER SORTING: \n");
    printDictionary(dictionary);

    printf("\nTEST RESULTS: \n");
    checkTextFile(fpText, dictionary);

    printf("\nUPDATED VERSION OF DICTIONARY (Words are sorted realtime according to their hit counts):\n");
    printDictionary(dictionary);

    return 0;
}


/*
*   FUNCTION: checkTextFile
*   @param1 fp: Pointer to the file that will be checked for misspellings.
*   @param2 dictionary: Pointer to the dictionary
*   @returns nothing
*
*   INFO: Checks if a file has incorrect (misspelled) words in it using a dictionary.
*         Updates the dictionary so that the most used words will be checked first.
*         Prints the results to the standard output.
*/
void checkTextFile(FILE* fp, arrayList** dictionary) {

    char* word;
    int counter=1; 

    while(!feof(fp)) {

        word = readWordFromFile(fp);
        if(strlen(word) > 0) {
            
            if(checkWord(word, dictionary) == false) {
                #ifdef __linux
                    printf("Incorrect word detected at %d. word in the file:\x1B[31m %s\n\x1B[0m",counter,word);
                #elif
                    printf("--!--Incorrect word detected at %d. word in the file: %s\n",counter,word);
                #endif
            }
            
        }
        counter++;

    }
    printf("File check is completed.\n");
}

/*
*   FUNCTION: checkWord
*   @param1 word: Pointer to the string to be checked 
*   @param2 dictionary: Pointer to the dictionary
*   @returns true if the word is found in the dictionary, false otherwise
*
*   INFO: Checks whether a word is in the dictionary (i.e. a valid word).
*/

bool checkWord(char* word, arrayList** dictionary) {

    arrayList* list = dictionary[ indexOfChar(word[0]) ];
    if(list == NULL) {
        return false;
    }

    // First search the word in the most accessed words.
    int i=0;
    while( (i < list->wordCount) && (list->accessCount[i] != 0) ) {
        if(stricmp(list->words[i], word) == 0) {
            
            arrayListTouchWord(list, i); // Increment the access count for the word.

            #ifdef ACCESSCOUNT_DEBUG
            printf("Word \"%s\" is a hit. Most accessed words beginning with letter '%c' are now: \n",word, word[0]);
            arrayListPrintMostAccessed(list);
            #endif
           
            return true;
        }
        i++;
    }
    // End of the list, but no match
    if(i == list->wordCount) {
        return false;
    }
    // Else; list still has words with 0 access counts:

    // Search the rest of the list using binary search algorithm
    return arrayListBinarySearch(list, i, list->wordCount - 1, word);
}


/*
*   FUNCTION: quickSortDictionary
*   @param1 dictionary: Pointer to the dictionary 
*   @returns nothing
*
*   INFO: Sorts the entire dictionary in an alphabetical order (A->Z)
*         using quicksort algorithm. (case-insensitive)
*/
void quickSortDictionary(arrayList** dictionary) {

    int i;
    for(i=0; i < 26; i++) {
        if(dictionary[i] != NULL) {
            arrayListQuickSort(dictionary[i], 0, dictionary[i]->wordCount - 1);
        }
    }
}

/*
*   FUNCTION: printDictionary
*   @param1 dictionary: Pointer to the dictionary 
*   @returns nothing
*
*   INFO: Prints the content of the dictionary to the standard output.
*/
void printDictionary(arrayList** dictionary) {
    int i;
    for(i=0; i < 26; i++) {
        if(dictionary[i] != NULL) {
            arrayListPrintHorizontal(dictionary[i]);
        }
    }
}

/*
*   FUNCTION: fillDictionary
*   @param1 dictionary: Address of the dictionary to be filled with words
*   @param2 fp: Pointer to the file to read words from
*   @returns nothing (@param1 is used as the return parameter)
*
*   INFO: Reads the given file and fills the dictionary list.
*/
void fillDictionary(arrayList*** dictionary, FILE* fp) {

    while(!feof(fp)) {

        char* line = readWordFromFile(fp);
        if(strlen(line) > 0) {
            
            int index = indexOfChar(line[0]);

            // If there is no arraylist created for this word's first letter yet, create it.
            if((*dictionary)[ index ] == NULL) {
                (*dictionary)[ index ] = arrayListCreate(2);
            }
            // Add the word to the list
            arrayListAdd((*dictionary)[ index ], line);

        }
    }
}


/*
*   FUNCTION: indexOfChar
*   @param1 ch: The character
*   @returns the given character's index
*
*   INFO: Finds the given character's alphabetical order, 
*         starting from a/A = 0 (case-insensitive)
*/
int indexOfChar(char ch) {

    // if char is a lowercase letter
    if(ch >= 'a') {
        return ch - 'a';
    }
    // if char is an uppercase letter
    return ch - 'A';
    
}

/*
*   FUNCTION: openFiles
*   @param1 fpDictionary: Address of the file pointer to be assigned as the dictionary
*   @param2 fpText: Address of the file pointer to be assigned as the text
*   @param3 fnDictionary: Path of the dictionary file   
*   @param4 fnText: Path of the text file
*   @returns nothing (@param1 and @param2 are used as the return parameters)
*
*   INFO: Opens necessary files and assigns their addresses to 
*         respective pointers while checking for errors.
*/
void openFiles(FILE** fpDictionary, FILE** fpText, const char* fnDictionary, const char* fnText) {
    
    *fpDictionary = fopen(fnDictionary, "r");
    
    if(*fpDictionary == NULL) {
        printf("Error reading file: %s\n", fnDictionary);
        exit(0);
    }

    *fpText = fopen(fnText, "r");
    
    if(*fpText == NULL) {
        printf("Error reading file: %s\n", fnText);
        exit(0);
    }

}


/*
*   FUNCTION: readWordFromFile
*   @param1 fp: Pointer of the file to be read
*   @returns the pointer to the scanned string
*
*   INFO: Securely reads a single word that is seperated by an
*         empty space or a newline from the given file without 
*         causing buffer overflow.
*/
char* readWordFromFile(FILE* fp) {

    char* buffer = (char*) malloc( sizeof(char) * AVG_WORD_LEN);

    if(buffer == NULL) {
        printf("Memory allocation error while reading a file stream.\n");
        exit(0);
    }

    buffer[0] = 0;
    int currentMax = AVG_WORD_LEN;
    int currentLen = 0;
    char ch = fgetc(fp); // Read first char

    // Read the stream char by char until the end of line or until the end of file.
    while( (ch != '\n') && (ch != EOF) && (ch != 13) && (ch != ' ')) {
        // If the allocated space for the buffer is full, expand it.
        if(currentLen == currentMax) {
            currentMax *= 1.5;
            buffer = (char*) realloc(buffer, sizeof(char) * currentMax);
            if(buffer == NULL) {
                printf("Memory allocation error while reading a file stream. "
                       "(Couldn't extend buffer to %d bytes)\n", currentMax);
                exit(0);
            }
        }
        // Push the char to the buffer
        buffer[currentLen] = ch;
        currentLen++;
        // Read next char
        ch = fgetc(fp); 
    }


    // Terminate the string with string terminator.
    if(currentLen == currentMax) {
        currentMax++;
        buffer = (char*) realloc(buffer, sizeof(char) * currentMax);
        if(buffer == NULL) {
            printf("Memory allocation error while reading a file stream. "
                    "(Couldn't extend buffer to %d bytes)\n", currentMax);
            exit(0);
        }
    }
    buffer[currentLen] = 0; // 0 = string terminator

    return buffer;

}


/*
*   ____________________
*   ARRAYLIST FUNCTIONS
*   ____________________
*
*/

/*
*   FUNCTION: arrayListCreate
*   @param1 initialSize: Initial size to be allocated
*   @returns arrayList pointer
*
*   INFO: Creates an arraylist struct and returns its pointer.
*/
arrayList* arrayListCreate(int initialSize) {

    arrayList* list = (arrayList*) malloc( sizeof(arrayList) );

    if(list == NULL) {
        printf("Memory allocation error while creating an arraylist\n");
        exit(0);
    }

    list->words = (char**) malloc( sizeof(char*) * initialSize);

    if(list->words == NULL) {
        printf("Memory allocation error while creating an arraylist.\n");
        exit(0);
    }

    list->accessCount = (int*) malloc( sizeof(int) * initialSize);
    if(list->accessCount == NULL) {
        printf("Memory allocation error while creating an arraylist.\n");
        exit(0);
    }

    list->size = initialSize;
    list->wordCount = 0;

    return list;

}


/*
*   FUNCTION: arrayListAdd
*   @param1 list: Pointer of the arraylist to add to
*   @param2 word: Pointer of the char array to be added
*   @returns nothing
*
*   INFO: Adds the given word to the given list.
*/
void arrayListAdd(arrayList* list, char* word) {

    // If the allocated size is full, expand it.
    if(list->wordCount == list->size) {
        
        int newSize = list->size * SIZE_MULTIPLIER;
        list->size = newSize;
        list->words = (char**) realloc(list->words, sizeof(char*) * newSize);

        if(list->words == NULL) {
            printf("Memory allocation error while adding an element to the arraylist."
                   "(Couldn't extend the size of the arraylist to %d bytes)", newSize);
            exit(0);
        }

        list->accessCount = (int*) realloc(list->accessCount, sizeof(int) * newSize);

        if(list->accessCount == NULL) {
            printf("Memory allocation error while adding an element to the arraylist."
                   "(Couldn't extend the size of the arraylist to %d bytes)", newSize);
            exit(0);
        }

    } 

    // Place the word at the end of the list.
    list->words[list->wordCount] = word;
    list->accessCount[list->wordCount] = 0;
    list->wordCount++;

}


/*
*   FUNCTION: arrayListPrintHorizontal
*   @param1 list: Pointer of the arraylist to be printed
*   @returns nothing
*
*   INFO: Prints the content of the given arraylist to 
*         the standard output horizontally
*/
void arrayListPrintHorizontal(arrayList* list) {

    if(list->wordCount > 0) {

        printf("%s", list->words[0]);
        int i;
        for(i=1; i < list->wordCount; i++) {
            printf(" - %s", list->words[i]);
        }

        printf("\n");
    }

}

/*
*   FUNCTION: arrayListPrintMostAccessed
*   @param1 list: Pointer of the arraylist to be printed
*   @returns nothing
*
*   INFO: Prints the most accessed words in a given arraylist in a descending order.
*         Words with 0 access will not be printed.
*/
void arrayListPrintMostAccessed(arrayList* list) {
    
    if(list->accessCount[0] != 0) {
        printf("%s(%d)", list->words[0], list->accessCount[0]);
    }
    
    int i=1;
    while(list->accessCount[i] != 0) {
        printf(" - %s(%d)", list->words[i], list->accessCount[i]);
        i++;
    }
    printf("\n");

}

/*
*   FUNCTION: arrayListTouchWord
*   @param1 list: Pointer to the arraylist
*   @param2 index: Index of the element in the arraylist that will be updated
*   @returns nothing
*
*   INFO: Increments the access count of the word with the given index and
*         updates the position of the word based on its access count.
*/
void arrayListTouchWord(arrayList* list, int index) {
    list->accessCount[index]++; // Increment the access count.
    
    // As long as the element has a larger access count than the leftside elements,
    // swap the elements until it is correctly placed
    while( (index>0) && (list->accessCount[index] > list->accessCount[index-1]) )  {

        arrayListSwapElements(list, index, index-1);
        index--;
    }
}


/*
*   FUNCTION: arrayListSwapElements
*   @param1 list: Pointer to the arraylist
*   @param2 i: Index of the first element that is wanted to be swapped
*   @param3 j: Index of the second element that is wanted to be swapped
*   @returns nothing
*
*   INFO: Swaps the locations of the given 2 elements in the arraylist.
*/
void arrayListSwapElements(arrayList* list, int i, int j) {

    char* strTemp = list->words[i];
    int iTemp = list->accessCount[i];

    list->words[i] = list->words[j];
    list->accessCount[i] = list->accessCount[j];

    list->words[j] = strTemp;
    list->accessCount[j] = iTemp;

}

/*
*   FUNCTION: arrayListBinarySearch
*   @param1 list: Pointer to the arraylist
*   @param2 left: Leftmost index of the arraylist
*   @param3 right: Rightmost index of the arraylist
*   @param3 key: The string to search for
*   @returns true if the key is found in the arraylist, false otherwise
*
*   INFO: Searches a word in the arraylist using binary search algorithm.
*/
bool arrayListBinarySearch(arrayList* list, int left, int right, char* key) {

    if(left>right) {
        return false;
    }

    int middle = (left+right)/2;
    if(stricmp(list->words[middle], key) == 0) {

        arrayListTouchWord(list, middle); // Increment the access count for the word.

        #ifdef ACCESSCOUNT_DEBUG
        printf("Word \"%s\" is a hit. Most accessed words beginning with letter '%c' are now: \n",key, key[0]);
        arrayListPrintMostAccessed(list);
        #endif

        return true;
    }
    if(stricmp(list->words[middle], key) < 0) {
        return arrayListBinarySearch(list, middle+1, right, key);
    }
    return arrayListBinarySearch(list, left, middle-1, key);

}

/*
*   FUNCTION: arrayListQuickSort
*   @param1 list: Pointer of the arraylist to be sorted
*   @param2 left: Leftmost index of the array
*   @param3 right: Rightmost index of the array
*   @returns nothing
*
*   INFO: Sorts the array in an alphabetical order (A->Z) 
*         using quicksort algorithm. (case-insensitive)
*/
void arrayListQuickSort(arrayList* list, int left, int right) {
    if(left<right) {

        int pivot = partition(list, left, right);

        arrayListQuickSort(list, left, pivot-1);
        arrayListQuickSort(list, pivot+1, right);

    }
}

/*
*   FUNCTION: partition
*   @param1 list: Pointer of the arraylist to be partitioned
*   @param2 left: Leftmost index of the array
*   @param3 right: Rightmost index of the array
*   @returns the pivot (middle element)
*
*   INFO: Selects the leftmost element as pivot and places it at its
*         correct position. All smaller elements are placed to left of
*         the pivot and all greater elements are placed to the right of
*         the pivot.
*/
int partition(arrayList* list, int left, int right) {
    int pivot = left;
    int i = left;
    int j = right;

    do {
        do {
            i++;
        } while( i<j && ( stricmp(list->words[i], list->words[pivot]) < 0 ) );
        
        while( ( stricmp(list->words[pivot], list->words[j]) < 0 ) ) {
            j--;
        }

        if(i<j) {
            arrayListSwapElements(list, i, j);
            j--;
        }

    } while(i<j);

    arrayListSwapElements(list, left, j);
    return j;
}
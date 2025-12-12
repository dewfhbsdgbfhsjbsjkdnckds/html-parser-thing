#include <stdlib.h>
#include <string.h>

typedef struct{
	int capacity;
	int size;
	char *string;
} listString; 

// creates a null terminated arraylist string
// and allocates `capacity` bytes
listString createListString(int capacity){
	listString string = {capacity, 0, malloc(capacity)};
	if (capacity > 0) string.string[0] = 0;
	return string;
};

void addCharToListString(listString *string, char character){
	if (string->size + 1 == string->capacity){
		string->capacity *= 2;
		string->string = realloc(string->string, string->capacity);
	}
	string->string[string->size] = character;
	string->string[string->size + 1] = 0;
	string->size++;
}

void clearListString(listString *inputString){
	inputString->string[0] = 0;
	inputString->size = 0;
}

void addStringToListString(listString *listString, char *addedString){
	if (listString->size + strlen(addedString) + 1 > listString->capacity){

	}
}

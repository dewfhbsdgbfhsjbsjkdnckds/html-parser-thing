#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listString.c"

char *oldtext = "<!DOCTYPE html>\n\
<html>\n\
	<head>\n\
		<title>Title</title>\n\
	</head>\n\
\n\
	<body>\n\
		<h1>heading</h1>\n\
		<p>paragraph</p>\n\
	</body>\n\
</html>";

char *text = "<!DOCTYPE html>\n\
<html>\n\
	<head>\n\
		<title>Title</title>\n\
	</head>\n\
\n\
	<body>\n\
		<h1>heading</h1>\n\
		<p>im going to write a very very\n\
		big paragraph and its going to have a lot of words</p>\n\
	</body>\n\
</html>";

struct element {
	// maybe i change element type to be a string?
	// i dont know if html allows user defined types
	enum elementType {
		html,
		head,
		title,
		body,
		h1,
		p,
		comment,
		data // im not sure if i need a data element type but it seems handy
	} type;
	struct elementList {
		int capacity;
		int size;
		// list of pointers
		struct element **children;
	} childrenList;
	struct element *parent;
	// list of children
	char *data;
};
typedef struct element element;

typedef struct {
	element *html;
} document;

typedef struct {
	int capacity;
	int size;
	char **attributeName;
	char **value;
} attributeList;

typedef struct {
	enum tokentype {
		opentag,
		endTag,
		characterToken,
		doctypeToken,
		commentToken // not implenented yet
		// i might need eof token
		// but not right now
	} type;
	listString data;
	attributeList attributes;
} token;

typedef enum {
	initial,
	beforeHTML,
	beforeHead,
	inHead
} insertionMode;

void AddChild(struct elementList *list, element *newElement){
	if (list->size == list->capacity){
		list->capacity *= 2;
		list->children = realloc(list->children, list->capacity * sizeof(element *));
	}
	list->children[list->size] = newElement;
	list->size++;
}

insertionMode currentMode = initial;
element *currentElement;
document outputDocument;
// i have no idea how to do this lmao
void feedToken(token *inputToken){
	if (currentMode == initial){
		if (inputToken->type == characterToken){
			// ignore leading \n, \t, space
			if (inputToken->data.string[0] == '\n'){}
			else if (inputToken->data.string[0] == '\t'){}
			else if (inputToken->data.string[0] == ' '){}
			//else printf("character token fed: '%s'\n", inputToken->data.string);
			return;
		}
		if (inputToken->type == doctypeToken){
			printf("%s\n", inputToken->data.string);
			// problem here is that data.string is " html" not "html"
			if (!strncmp(inputToken->data.string, "html", 3)){
				printf("doctype data is: %s\n", inputToken->data.string);
				currentMode = beforeHTML;
				return;
			}
		}
	}
	if (currentMode == beforeHTML){
		if (inputToken->type == commentToken){
			// todo implement
		}
		// open tag <html>
		// a bit janky but im trying to stay a little closer to what the spec is
		if (inputToken->type == opentag && !strncmp(inputToken->data.string, "html", 4)){
			if (outputDocument.html == NULL){
				element *tempHtml = malloc(sizeof(element));
				struct elementList list = {16, 0, malloc(16 * sizeof(element *))};
				tempHtml->childrenList = list;
				tempHtml->type = html;
				outputDocument.html = tempHtml;
				currentElement = tempHtml;

				currentMode = beforeHead;
				return;
			}
			else {
				printf("error: 2 html open tags\n");
				return;
			}
		}
		else {
			element *tempHtml = malloc(sizeof(element));
			struct elementList list = {16, 0, malloc(16 * sizeof(element *))};
			tempHtml->childrenList = list;
			tempHtml->type = html;
			outputDocument.html = tempHtml;
			currentElement = tempHtml;

			currentMode = beforeHead;
			// no return here so it goes so beforeHead
		}
	}
	if (currentMode == beforeHead){
		if (inputToken->type == characterToken){
			// ignore leading \n, \t, space
			if (inputToken->data.string[0] == '\n'){}
			else if (inputToken->data.string[0] == '\t'){}
			else if (inputToken->data.string[0] == ' '){}
			return;
		}
		if (inputToken->type == opentag){
			if (!strncmp(inputToken->data.string, "head", 4)){
				element *newElement = malloc(sizeof(element));
				newElement->parent = currentElement;
				newElement->type = head;
				struct elementList list = {16, 0, malloc(16 * sizeof(element *))};
				newElement->childrenList = list;
				currentElement = newElement;
				currentMode = inHead;
				printf("made new head element\n");
			}
		}
	}
}

// redo this to work with a stream of bytes/chars, but thats annoying
// todo handle closing tags
// todo add more comments, explaining maybe
void parse(char *input){
	enum state {
		insideTag,
		tagname,
		dataState,
		rawtext,
		beforeAttributeName,
		endTagOpen,
		markupDeclarationOpen,
		bogusComment,
		DOCTYPE,
		beforeDOCTYPEname,
		DOCTYPEname
	} currentState;
	currentState = dataState;
	token currentToken;
	currentToken.data = createListString(32);
	listString buffer = createListString(128);
	for (int i = 0; input[i] != 0; i++){
		if (currentState == dataState){
			if (input[i] == '<'){
				currentState = insideTag;
				continue;
			}
			else {
				currentToken.type = characterToken;
				addCharToListString(&currentToken.data, input[i]);
				feedToken(&currentToken);
				clearListString(&currentToken.data);
				continue;
			}
		}
		// TAG OPEN STATE
		if (currentState == insideTag){
			if (isalpha(input[i])){
				currentState = tagname;
				i--;
				currentToken.type = opentag;
				continue;
			}
			else if (input[i] == '/'){
				currentState = endTagOpen;
				continue;
			}
			else if (input[i] == '!'){
				currentState = markupDeclarationOpen;
				continue;
			}
		}
		// TAG NAME STATE
		// <name attribute=value>
		// manages the name part
		if (currentState == tagname){
			if (isalpha(input[i])){
				// add char to current tokens tag name
				addCharToListString(&currentToken.data, input[i]);
				continue;
			}
			else if (input[i] == '>'){
				currentState = dataState;
				feedToken(&currentToken);
				clearListString(&currentToken.data);
				continue;
			}
			else if (input[i] == ' '){
				currentState = beforeAttributeName;
				continue;
			}
		}
		if (currentState == endTagOpen){
			currentToken.type = endTag;
			currentState = tagname;
			continue;
		}
		if (currentState == markupDeclarationOpen){
			addCharToListString(&buffer, input[i]);
			// checks if the any characters dont match, or if the string is too long
			if (!strcmp(buffer.string, "DOCTYPE")){
				currentState = DOCTYPE;
				continue;
			}
			else if (buffer.size > 7 || strncmp(buffer.string, "DOCTYPE", buffer.size)){
				currentState = bogusComment;
				continue;
			}
		}
		if (currentState == bogusComment){
			if (input[i] == '>'){
				currentState = dataState;
				continue;
			}
		}
		if (currentState == DOCTYPE){
			if (input[i] == ' '){
				currentState = beforeDOCTYPEname;
				continue;
			}
		}
		if (currentState == beforeDOCTYPEname){
			currentToken.type = doctypeToken;
			currentState = DOCTYPEname;
			addCharToListString(&currentToken.data, input[i]);
			continue;
		}
		if (currentState == DOCTYPEname){
			if (input[i] == '>'){
				currentState = dataState;
				feedToken(&currentToken);
				continue;
			}
			else {
				addCharToListString(&currentToken.data, input[i]);
				continue;
			}
		}
	}
}


int main(int argc, char *argv[]){
	document documentObject = {NULL};
	parse(text);
	printf("%s\n", text);
	return 0;
}

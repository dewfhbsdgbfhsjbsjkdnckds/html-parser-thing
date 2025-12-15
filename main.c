#include <ctype.h>
#include <stdio.h>
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

typedef struct{
	// maybe i change element type to be a string?
	// i dont know if html allows user defined types
	enum elementType {
		html,
		head,
		title,
		body,
		h1,
		p,
		data
	} type;
	struct element *children;
	char *data;
} element;

typedef struct {
	element *html;
} document;

// todo change to using a `document` struct
char rootMade = 0;
element root;

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
		doctype
		// i might need eof token
		// but right now im just using strings and strings dont really have eof,
		// they have null terminator
		// idk ill see how i end up implementing it later
	} type;
	listString data;
	attributeList attributes;
} token;

typedef enum {
	initial
} insertionMode;

void feedToken(token *inputToken, insertionMode *currentMode, document *outputDocument){
	if (*currentMode == initial){
		if (inputToken->type == characterToken){
			if (!strncmp(inputToken->data.string, "\n", 1)){}
			else if (!strncmp(inputToken->data.string, "\t", 1)){}
			else if (!strncmp(inputToken->data.string, " ", 1)){}
			//else printf("character token fed: '%s'\n", inputToken->data.string);
		}
		if (inputToken->type == doctype){
			printf("doctype data is: %s\n", inputToken->data.string);
		}
	}
	if (inputToken->type == opentag){
		if (!rootMade && !strcmp(inputToken->data.string, "html")){
			printf("detected html start tag\n");
			root.type = html;
			rootMade = 1;
			return;
		}
	}
	if (inputToken->type == endTag){
		if (!strcmp(inputToken->data.string, "head")){
			printf("detected head end tag\n");
		}
		else printf("detected other end tag\n");
	}
}

// redo this to work with a stream of bytes/chars, but thats annoying
// todo handle closing tags
// todo add more comments, explaining maybe
void parse(char *input, document *inputDocument){
	enum state {
		insideTag,
		tagname,
		dataState,
		rawtext,
		beforeAttributeName,
		endTagOpen,
		markupDeclarationOpen,
		bogusComment,
		beforeDOCTYPEname,
		DOCTYPEname
	} currentState;
	currentState = dataState;
	insertionMode tokenParsingMode = initial;
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
				feedToken(&currentToken, &tokenParsingMode, inputDocument);
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
				feedToken(&currentToken, &tokenParsingMode, inputDocument);
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
				currentState = beforeDOCTYPEname;
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
		if (currentState == beforeDOCTYPEname){
			currentToken.type = doctype;
			currentState = DOCTYPEname;
			addCharToListString(&currentToken.data, input[i]);
				continue;
		}
		if (currentState == DOCTYPEname){
			if (input[i] == '>'){
				currentState = dataState;
				feedToken(&currentToken, &tokenParsingMode, inputDocument);
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
	parse(text, &documentObject);
	printf("%s\n", text);
	return 0;
}

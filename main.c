#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "listString.c"

char *text = "<!DOCTYPE html>\n\
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

typedef struct{
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
		closetag,
		datatoken,
		doctype
	} type;
	listString data;
	attributeList attributes;
} token;

void feedToken(token *inputToken){
	if (inputToken->type == opentag){
		if (!rootMade && !strcmp(inputToken->data.string, "html")){
			printf("detected html start tag\n");
			root.type = html;
			rootMade = 1;
			return;
		}
		if (!strcmp(inputToken->data.string, "head")){
			printf("detected head start tag\n");
		}
		if (!strcmp(inputToken->data.string, "title")){
			printf("detected title start tag\n");
		}
		if (!strcmp(inputToken->data.string, "body")){
			printf("detected body start tag\n");
		}
		if (!strcmp(inputToken->data.string, "h1")){
			printf("detected h1 start tag\n");
		}
		if (!strcmp(inputToken->data.string, "p")){
			printf("detected p start tag\n");
		}
	}
	if (inputToken->type == closetag){

	}
	if (inputToken->type == datatoken){
		printf("data token fed: %s\n", inputToken->data.string);
	}
}

// redo this to work with a stream of bytes/chars, but thats annoying
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
				if (currentToken.data.size != 0){
					feedToken(&currentToken);
					clearListString(&currentToken.data);
				}
			}
			else if (input[i] != '\n'){
				currentToken.type = datatoken;
				addCharToListString(&currentToken.data, input[i]);
			}
		}
		// TAG OPEN STATE
		if (currentState == insideTag){
			if (isalpha(input[i])){
				printf("is alpha: %c\n", input[i]);
				currentState = tagname;
				i--;
				currentToken.type = opentag;
			}
			else if (input[i] == '/'){
				currentState = endTagOpen;
			}
			else if (input[i] == '!'){
				currentState = markupDeclarationOpen;
			}
		}
		// TAG NAME STATE
		// <name attribute=value>
		// manages the name part
		if (currentState == tagname){
			if (isalpha(input[i])){
				// add char to current tokens tag name
				addCharToListString(&currentToken.data, input[i]);
			}
			else if (input[i] == '>'){
				currentState = dataState;
				feedToken(&currentToken);
				clearListString(&currentToken.data);
			}
			else if (input[i] == ' '){
				currentState = beforeAttributeName;
			}
		}
		if (currentState == markupDeclarationOpen){
			addCharToListString(&buffer, input[i]);
			// checks if the any characters dont match, or if the string is too long
			if (buffer.size > 7 || strncmp(buffer.string, "DOCTYPE", buffer.size)){
				currentState = bogusComment;
			}
			else if (!strcmp(buffer.string, "DOCTYPE")){
				currentState = beforeDOCTYPEname;
			}
		}
		if (currentState == bogusComment){
			if (input[i] == '>'){
				currentState = dataState;
			}
		}
		if (currentState == beforeDOCTYPEname){
			currentToken.type = doctype;
			currentState = DOCTYPEname;
			addCharToListString(&currentToken.data, input[i]);
		}
		if (currentState == DOCTYPEname){
			if (input[i] == '>'){
				currentState = dataState;
			}
			else {
				addCharToListString(&currentToken.data, input[i]);
			}
		}
	}
}


int main(int argc, char *argv[]){
	parse(text);
	printf("%s\n", text);
	return 0;
}

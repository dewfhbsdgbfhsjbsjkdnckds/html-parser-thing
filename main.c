#include <ctype.h>
#include <stdio.h>
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

element parse(char *input){
	element html = {0, NULL, NULL};
	enum state {
		insideTag,
		tagname,
		data,
		rawtext,
		beforeAttributeName
	} currentState;
	currentState = data;
	listString buffer = createListString(128);
	for (int i = 0; input[i] != 0; i++){
		if (currentState == data){
			if (input[i] == '<'){
				currentState = insideTag;
			}
		}
		if (currentState == insideTag){
			if (isalpha(input[i])){
				currentState = tagname;
				i--;
			}
		}
		// <name attribute=value>
		// manages the name part
		if (currentState == tagname){
			if (isalpha(input[i])){
				addCharToListString(&buffer, input[i]);
			}
			else if (input[i] == '>'){
				currentState = data;
			}
			else if (input[i] == ' '){
				currentState = beforeAttributeName;
			}
		}

	}
	return html;
}

int main(int argc, char *argv[]){
	element html = parse(text);
	printf("%s\n", text);
	return 0;
}

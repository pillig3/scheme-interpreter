#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "value.h"
#include "tokenizer.h"
#include "talloc.h"
#include "linkedlist.h"
#include <assert.h>

/*
* Adds a Value of type BOOL_TYPE, OPEN_TYPE, or CLOSE_TYPE to the given list,
* as specified via the parameter 'type'.
* For boolean tokens, updates the string to be #t or #f as specified by b.
* For open or closed tokens, b is treated as a dummy variable and is ignored.
*/
Value *addBoolToken(Value *list, int type, bool b) {
    Value *newEntry = makeNull();
    newEntry->type = type;
    if (type == BOOL_TYPE) {
        newEntry->b = b;
    }
    return cons(newEntry,list);
}

/*
* Adds a Value of STR_TYPE to the given list and
* returns the resulting list.
* This function is used to collate characters so that
* we can store the corresponding string or symbol token later.
*/
Value *addStrToken(Value *list, char c) {
    Value *newEntry = makeNull();
    newEntry->type = STR_TYPE;
    newEntry->s = talloc(sizeof(char)+1);
    (newEntry->s)[0] = c;
    return cons(newEntry,list);
}

/*
* Adds a Value of SYMBOL_TYPE to the given list and
* returns the resulting list.
*/
Value *addSymbolToken(Value *list, char *s, int length) {
    Value *newEntry = makeNull();
    newEntry->type = SYMBOL_TYPE;
    newEntry->s = talloc((length) * sizeof(char) + 1);
    strcpy(newEntry->s, s);
    return cons(newEntry,list);
}

/*
* Checks ascii code to see if c is 0,1,..., or 9
*/
bool isDigit(char c) {
    return ( (int)'0' <= (int)c && (int)c <= (int)'9' );
}

/*
* Checks ascii code to see if c is a letter
*/
bool isLetter(char c) {
    return (( (int)'a' <= (int)c && (int)c <= (int)'z'  ) ||
            ( (int)'A' <= (int)c && (int)c <= (int)'Z' ));
}

/*
* Returns if given character can be at the start of a symbol
*/
bool isInitial(char c) {
  return (
      isLetter(c) || c == '!' || c == '$' || c == '%' ||
      c == '&' || c == '*' || c == '/' || c == ':' ||
      c == '<' || c == '=' || c == '>' || c == '?' ||
      c == '~' || c == '_' || c == '^' );
}

/*
* Returns if given character can be in the middle of a symbol
*/
bool isSubsequent(char c) {
    return (isInitial(c) || isDigit(c) || c == '.' || c == '+' || c == '-');
}

/*
* Converts a linkedlist of Values with string types into a single value
* with string type, which is the concatenation of the Values' strings
*/
Value *strListToVal(Value *list, int length, int type) {
    Value *strValue = makeNull();
    strValue->type = type;
    strValue->s = talloc((length * sizeof(char))+1);
    // initialize to be array of null characters to avoid valgrind errors
    for (int j=0; j<length+1; j++) {
        (strValue->s)[j] = '\0';
    }
    int i = 0;
    list = reverse(list);
    while ((list->type) != NULL_TYPE) {
        assert(car(list)->s);
        (strValue->s)[i] = *(car(list)->s);
        i++;
        list = cdr(list);
    }
    return strValue;
}

/*
* Called to exit the tokenizer and print an error message.
*/
void exitWithError(Value *list, int lineNum) {
    if (list->type == NULL_TYPE) {
        printf("Error: invalid syntax on line %i.\n", lineNum);
        texit(1);
    }
    printf("Error: invalid syntax on line %i, after tokens: \n", lineNum);
    Value *lastfew = makeNull();
    for (int i = 0; i < 10; i++) {
        if (list->type != NULL_TYPE) {
            lastfew = cons(car(list), lastfew);
            list = cdr(list);
        }
    }
    displayTokens(lastfew);
    texit(1);
}

/*
* Adds a string to the list
*/
Value *addString(Value *list, char charRead, int lineNum) {
    Value *strList = makeNull();
    int length = 1;
    charRead = fgetc(stdin);
    while (charRead!='\"') {
        if (charRead == EOF) {
            exitWithError(list, lineNum);
        } else if (charRead == '\\') {
            // Escaped character.
            charRead = fgetc(stdin);
            switch (charRead) {
              case 'n':
                charRead = '\n';
                break;
              case '\"':
                charRead = '\"';
                break;
              case '\'':
                charRead = '\'';
                break;
              case 't':
                charRead = '\t';
                break;
              case '\\':
                charRead = '\\';
                break;
              default:
                exitWithError(list, lineNum);
            }
        }
        strList = addStrToken(strList, charRead);
        charRead = fgetc(stdin);
        length++;
    }
    return cons(strListToVal(strList, length, STR_TYPE), list);
}

/*
* Adds a DOUBLE_TYPE token to the list
*/
Value *addDouble(Value *list, double d, char charRead,
                 char oldChar, int lineNum) {
    int numDec = -1; // counter
    while (isDigit(charRead)) {
        // After the decimal point, multiply the n^th digit by
        // 10^(-n) and then add it to the result d.
        d = d + (charRead - '0') * pow(10, numDec);
        numDec--;
        charRead = fgetc(stdin);
    }
    if (!(charRead == '\n' || charRead == '(' || charRead == ')' ||
          charRead == ' ' || charRead == EOF)) {
        exitWithError(list,lineNum);
    }
    Value *num = makeNull();
    num->type = DOUBLE_TYPE;
    if (oldChar == '-') {
        num->d = -(d);
    } else {
        num->d = d;
    }
    ungetc(charRead,stdin);
    return cons(num, list);
}

/*
* Adds an INT_TYPE token to the list
*/
Value *addInt(Value *list, int i, char oldChar, int lineNum) {
    Value *num = makeNull();
    num->type = INT_TYPE;
    if (oldChar == '-') {
        num->i = -(i);
    } else {
        num->i = i;
    }
    return cons(num, list);
}

/*
* Adds a number (or possibly + or -) to the list
*/
Value *addNumber(Value *list, char charRead, int lineNum) {
    bool isNum = true;
    char oldChar = charRead;
    if (charRead == '+' || charRead == '-'){
        charRead = fgetc(stdin);
        if (!(isDigit(charRead) || charRead == '.')) {
            char str[2];
            str[0] = oldChar;
            str[1] = '\0';
            list = addSymbolToken(list, str, 1);
            isNum = false;
            ungetc(charRead,stdin);
        }
    }
    if (charRead == '.') {
        charRead = fgetc(stdin);
        if (!(isDigit(charRead))) {
            isNum = false;
            if (charRead == ' ') {
                while (charRead == ' ' || charRead == '\n') {
                    charRead = fgetc(stdin);
                }
                if (charRead == EOF || charRead == ')') {
                    exitWithError(list,lineNum);
                }
                ungetc(charRead,stdin);
                Value *val = makeNull();
                val->type = DOT_TYPE;
                return cons(val, list);
            } else {
                exitWithError(list,lineNum);
            }
        }
        ungetc(charRead,stdin);
        charRead = '.';
    }
    if (isNum) {
        if (charRead == '.') {
            charRead = fgetc(stdin);
            if (!(isDigit(charRead))) {
                exitWithError(list,lineNum);
            }
            ungetc(charRead,stdin);
            charRead = '.'; // reassign so we enter double case
        }
        int i = 0;
        // Before decimal point, multiply the old result by 10
        // and add the next digit every loop.
        while (isDigit(charRead)) {
            i = i*10 + (charRead-'0'); // subtracting '0' converts char into int
            charRead = fgetc(stdin);
        }
        if (!(charRead == '\n' || charRead == '(' || charRead == '.' ||
              charRead == ')' || charRead == ' ' || charRead == EOF)) {
            exitWithError(list,lineNum);
        }
        if (charRead == EOF) {
            list = addInt(list, i, oldChar, lineNum);
            ungetc(charRead,stdin);
            return list;
        }
        if (charRead == '.') {
            charRead = fgetc(stdin);
            list = addDouble(list, (double) i, charRead, oldChar, lineNum);
        } else {
            list = addInt(list, i, oldChar, lineNum);
            ungetc(charRead,stdin);
        }
    }
    return list;
}

/*
* Adds a symbol to the list
*/
Value *addSymbol(Value *list, char charRead, int lineNum) {
    if (charRead == '\'') {
        Value *val = makeNull();
        val->type = SYMBOL_TYPE;
        val->s = "\'";
        return cons(val, list);
    }
    int length = 1;
    Value *strList = makeNull();
    strList = addStrToken(strList, charRead);
    charRead = fgetc(stdin);
    while (isSubsequent(charRead) && charRead != EOF) {
        if (charRead == '\\') {
            strList = addStrToken(strList, charRead);
            charRead = fgetc(stdin);
            length++;
            assert(charRead != EOF);
        }
        strList = addStrToken(strList, charRead);
        charRead = fgetc(stdin);
        length++;
   }
   ungetc(charRead, stdin);
   return cons(strListToVal(strList, length, SYMBOL_TYPE), list);
}

/*
* Reads the input stream and returns a linked list containing all tokens
* Uses bool activeInput to determine how to treat \n
*/
Value *tokenize(bool activeInput) {
    char charRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);
    int lineNum = 1; // current line, for error messages
    while (charRead != EOF) {
        if (charRead == '(') {
            list = addBoolToken(list, OPEN_TYPE, true);
        } else if (charRead == ')') {
            list = addBoolToken(list, CLOSE_TYPE, true);
        } else if (charRead == '#'){
            char result = fgetc(stdin);
            if (!(result == 't' || result == 'f')) {
                exitWithError(list, lineNum);
            }
            charRead = fgetc(stdin);
            // catch stuff like #tofu
            if (!((charRead == ' ')||(charRead == '\n') || (charRead == '(') ||
               (charRead == ')'))) {
                exitWithError(list,lineNum);
            }
            ungetc(charRead,stdin);
            list = addBoolToken(list, BOOL_TYPE, (result == 't'));
        } else if (charRead == '\"') {
            list = addString(list, charRead, lineNum);
        } else if (charRead == ';') {
            while (charRead != '\n' && charRead != EOF) {
                charRead = fgetc(stdin);
            }
            ungetc(charRead, stdin); // move back one place in input stream
        } else if (isDigit(charRead) || charRead == '+' ||
                   charRead == '-' || charRead == '.') {
            list = addNumber(list, charRead, lineNum);
        } else if (isInitial(charRead) || charRead == '\'') {
            list = addSymbol(list, charRead, lineNum);
        } else if (charRead == ' '){
        } else if (charRead == '\n') {
            lineNum++;
        } else {
            exitWithError(list, lineNum);
        }
        // treat \n as EOF for REPL loop to trigger next user input
        if (activeInput && charRead == '\n') {
            charRead = EOF;
        } else {
            charRead = fgetc(stdin);
        }
    }
    return reverse(list);
}

/*
* Given a list of tokens, prints out all tokens.
* On each line (one token/line), prints token:token_type
*/
void displayTokens(Value *list) {
    Value *next = list;
    assert(next);
    assert(next -> type);
    while ((next -> type) != NULL_TYPE) {
        Value *val = (next->c).car;
        assert(val);
        switch (val->type) {
            case INT_TYPE:
                printf("%i:integer\n", val->i);
                break;
            case DOUBLE_TYPE:
                printf("%f:double\n", val->d);
                break;
            case STR_TYPE:
                printf("%s:string\n", val->s);
                break;
            case BOOL_TYPE:
                printf("%s:boolean\n", (val->s ? "#t" : "#f"));
                break;
            case OPEN_TYPE:
                printf("(:open\n");
                break;
            case CLOSE_TYPE:
                printf("):close\n");
                break;
            case SYMBOL_TYPE:
                assert(val->s);
                printf("%s:symbol\n", val->s);
                break;
            case DOT_TYPE:
                printf(".:dot\n");
                break;
            default:
                printf("I don't know how to display this token type\n");
        }
        next = cdr(next);
        assert(next);
        assert(next->type);
    }
}

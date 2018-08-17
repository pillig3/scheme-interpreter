#include <stdlib.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"

#ifndef TOKENIZER_H
#define TOKENIZER_H

/*
* Reads the input stream and returns a linked list containing all tokens found.
* Also includes a boolean activeInput, which is true if the REPL is active and
* false otherwise.
*/
Value *tokenize(bool activeInput);

/*
* Given a list of tokens, prints out all tokens.
* On each line (one token/line), prints token:token_type
*/
void displayTokens(Value *list);

#endif

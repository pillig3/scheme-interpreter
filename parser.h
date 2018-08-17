#include <stdlib.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"

#ifndef PARSER_H
#define PARSER_H

/*
* Given a linked list of Scheme tokens returns a pointer to the 
* corresponding parse tree
*/
Value *parse(Value *tokens);

/*
* Given a parse tree, prints the tree using Scheme structure.
*/
void printTree(Value *tree);

#endif

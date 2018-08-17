#include <stdlib.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "parser.h"

#ifndef INTERPRETER_H
#define INTERPRETER_H

struct Frame {
    Value *bindings;
    struct Frame *parent;
};
typedef struct Frame Frame;

/*
* Makes and returns a frame with NULL parent.
*/
Frame *makeFrame();

/*
* Given a list of S-expressions (i.e., the output of parser), calls eval on
* each S-expression in the top-level environment. Prints the result of each
* eval.
*/
void interpret(Value *tree, Frame *frame);

/*
* Given a parse tree of a single S-expression and an environment frame,
* returns a pointer to a Value represented the expression's value.
*/
Value *eval(Value *expr, Frame *frame);

#endif

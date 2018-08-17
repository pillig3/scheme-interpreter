#include <stdlib.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/**********
countParen and append are helper functions for the REPL loop.
***********/

/*
* Given a list of tokens, returns the number of open parenthesis minus
* the number of closed parenthesis.
*/
int countParen(Value *list) {
    int count = 0;
    Value *item;
    while (list->type != NULL_TYPE) {
        item = car(list);
        list = cdr(list);
        if (item->type == OPEN_TYPE) {
            count += 1;
        } else if (item->type == CLOSE_TYPE) {
            count -= 1;
        }
    }
    return count;
}

/*
* Given a list of lists, return the appended version of the lists.
* If given a single item, return that item.
*/
Value *append(Value *arg1, Value *arg2) {
    assert(arg1->type == CONS_TYPE || arg1->type == NULL_TYPE);
    assert(arg2->type == CONS_TYPE || arg2->type == NULL_TYPE);
    Value *argTail = makeNull();
    while (arg1->type != NULL_TYPE) {
        argTail = cons(car(arg1), argTail);
        arg1 = cdr(arg1);
    }
    while (argTail->type == CONS_TYPE && car(argTail)->type != NULL_TYPE) {
        arg2 = cons(car(argTail), arg2);
        argTail = cdr(argTail);
    }
    return arg2;
}

int main(void) {
    int t = isatty(0);
    Frame *frame = makeFrame();
    if (t) {
        // isatty is true: want interactive loop
        printf("> ");
        char charRead = fgetc(stdin);
        Value *prevList = makeNull();
        while (charRead != EOF) {
            ungetc(charRead, stdin);
            Value *list = tokenize(true);
            // store past lines to eval multi-line commands
            prevList = append(prevList, list);
            int count = countParen(prevList); 
            if (count <= 0) {
                Value *tree = parse(prevList);
                interpret(tree, frame);
                prevList = makeNull();
                count = 0;
            }
            printf("> ");
            for (int i=0; i<count; i++) {
                printf("   ");
            }
            charRead = fgetc(stdin);
        }
    } else {
        // isatty is false: proceed like normal
        Value *list = tokenize(false);
        Value *tree = parse(list);
        interpret(tree, frame);
    }
    tfree();
    return 0;
}

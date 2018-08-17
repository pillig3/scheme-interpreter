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
* Called to exit the parser and print an error message.
*/
void exitParserWithError(char *message) {
    printf("%s\n",message);
    texit(1);
}

/*
* This function is called when a closed parenthesis is encountered.
* Pops items off of the stack until an open parenthesis is found.
* If the stack is empty, throws an error.
* If successful, pops the new list containing all items found between
* the parenthesis pair back onto the tree.
*/
Value *popItems(Value *tree) {
    if(tree->type != CONS_TYPE || !(car(tree))) {
        exitParserWithError("Syntax error: too many closed parentheses\n");
    }
    Value *newEntry = makeNull();
    Value *popped = car(tree);
    tree = cdr(tree);
    while (! (popped->type == OPEN_TYPE)) {
        newEntry = cons(popped,newEntry);
        if (tree->type != CONS_TYPE || !(car(tree))) {
            exitParserWithError("Syntax error: too many closed parentheses\n");
        }
        popped = car(tree);
        tree = cdr(tree);
    }
    return cons(newEntry, tree);
}

/*
* Called after reading a '. Adds the next datum to the tree,
* and then replaces 'datum with (quote datum), as specified at
* https://docs.racket-lang.org/reference/reader.html#%28part._parse-quote%29
* After this, returns the tree along with the remaining tokens:
* Specifically, returns (resulting tree, remaining tokens).
*/
Value *addQuasiquoted(Value *tree, Value *next, Value *tokens) {
    Value *open = makeNull();
    open->type = OPEN_TYPE;
    int openCount = 0;
    // add ( and quote
    tree = cons(open, tree);
    next->s = "quote";
    tree = cons(next, tree);
    if(tokens->type == NULL_TYPE){
        exitParserWithError("Syntax error: missing datum after a single quote\n");
    }
    next = car(tokens);
    tokens = cdr(tokens);
    
    if (next->type == OPEN_TYPE) {
        // if the next token is (, then add things until we reach the close paren
        tree = cons(next, tree);
        openCount++;
        while (openCount != 0) {
            if (tokens->type == NULL_TYPE) {
                exitParserWithError("Syntax error: not enough close parentheses\n");
            }
            next = car(tokens);
            tokens = cdr(tokens);
            if (next->type != CLOSE_TYPE) {
                if (next->type == OPEN_TYPE) {
                    openCount++;
                }
                if (next->type == SYMBOL_TYPE && !strcmp(next->s, "\'")) {
                    // If we run across a single quote, recurse.
                    Value *returnPair = makeNull();
                    returnPair = addQuasiquoted(tree, next, tokens);
                    tree = car(returnPair);
                    tokens = car(cdr(returnPair));
                } else {
                    tree = cons(next, tree);
                }
            } else {
                openCount--;
                tree = popItems(tree);
            }
        }
    } else {
        tree = cons(next, tree);
    }
    tree = popItems(tree); // add the last )
    Value *returnPair = makeNull();
    returnPair = cons(tokens, returnPair);
    returnPair = cons(tree, returnPair);
    return returnPair;
}


/*
* Given a linked list of Scheme tokens returns a pointer to the
* corresponding parse tree
*/
Value *parse(Value *tokens) {
    Value *tree = makeNull();
    if (!tokens || tokens->type == NULL_TYPE) {
        return tree;
    }
    Value *next = car(tokens);
    tokens = cdr(tokens);
    int openCount = 0;
    while (next->type != NULL_TYPE) {
        if (! (next->type == CLOSE_TYPE)) {
            if (next->type == OPEN_TYPE) {
                openCount++;
            }
            if (next->type == SYMBOL_TYPE && !strcmp(next->s, "\'")) {
                // Handles a single quote, replacing 'a with (quote a).
                Value *returnPair = makeNull();
                returnPair = addQuasiquoted(tree, next, tokens);
                tree = car(returnPair);
                tokens = car(cdr(returnPair));
            } else {
                tree = cons(next, tree);
            }
        } else {
            openCount--;
            tree = popItems(tree);
        }
        if (tokens->type != NULL_TYPE) {
            next = car(tokens);
            tokens = cdr(tokens);
        } else {
            next = makeNull();
        }
    }
    if (! (openCount == 0)) {
        exitParserWithError("Syntax error: not enough close parentheses\n");
    }
    return reverse(tree);
}

/*
* Prints an integer followed by a space, unless the integer is followed by
* a closed parenthesis, in which case it prints just the integer.
*/
void printInt(int i, Value *tree) {
    if (tree->type == CONS_TYPE) {
        printf("%i ", i);
    } else {
        printf("%i", i);
    }
}

/*
* Prints a double followed by a space, unless the double is followed by
* a closed parenthesis, in which case it prints just the double.
*/
void printDouble(double d, Value *tree) {
    if (tree->type == CONS_TYPE) {
        printf("%f ", d);
    } else {
        printf("%f", d);
    }
}

/*
* Prints a string or symbol followed by a space, unless the item is
* followed by a closed parenthesis, in which case it prints just the item.
*/
void printStr(char *s, Value *tree) {
    if (tree->type == CONS_TYPE) {
        printf("%s ", s);
    } else {
        printf("%s", s);
    }
}

/*
* Prints a boolean followed by a space, unless the item is
* followed by a closed parenthesis, in which case it prints just the item.
*/
void printBool(bool b, Value *tree) {
    if (tree->type == CONS_TYPE) {
        printf("%s ", b ? "#t" : "#f");
    } else {
        printf("%s", b ? "#t" : "#f");
    }
}

/*
* Prints a closed parenthesis followed by a space, unless it is followed by
* another closed parenthesis, in which case it prints ).
*/
void printClose(Value *tree) {
    if (tree->type == CONS_TYPE) {
        printf(") ");
    } else {
        printf(")");
    }
}

/*
* This function is called when the current value is null but when the
* tree itself is not. In other words, this function is called when we
* need to print (). After printing appropriately, the function reassigns
* next and tree via pointers.
*/
void printEmptyCase(Value *next, Value *tree) {
    printf("(");
    printClose(tree);
    if (tree->type == CONS_TYPE) {
        next = car(tree);
        tree = cdr(tree);
    }
}
/*
* Given a parse tree, prints the tree using Scheme structure.
*/
void printTree(Value *tree) {
    if (tree->type == NULL_TYPE) {
        return;
    }
    Value *next = makeNull();
    next = car(tree);
    tree = cdr(tree);
    if (next->type == NULL_TYPE) {
        printEmptyCase(next, tree);
    }
    while (!((next->type == NULL_TYPE) && (tree->type == NULL_TYPE))) {
        bool end = false;
        if (next->type == CONS_TYPE) {
            printf("(");
            printTree(next);
            printClose(tree);
        } else if (next->type == STR_TYPE || next->type == SYMBOL_TYPE) {
            printStr(next->s, tree);
        } else if (next->type == BOOL_TYPE) {
            printBool(next->b, tree);
        } else if (next->type == INT_TYPE) {
            printInt(next->i, tree);
        } else if (next->type == DOUBLE_TYPE) {
            printDouble(next->d, tree);
        } else if (next->type == DOT_TYPE) {
            printf(". ");
        } else {

        }
        if (tree->type == CONS_TYPE) {
            next = car(tree);
            tree = cdr(tree);
        } else {
            end = true;
            next = makeNull();
        }
        if (next->type == NULL_TYPE && !end) {
            printEmptyCase(next, tree);
        }

    }

}

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "value.h"
#include "tokenizer.h"
#include "talloc.h"
#include "linkedlist.h"
#include "parser.h"

int main(void) {
    Value *list = tokenize(stdin);
    Value *tree = parse(list);
    printTree(tree);
    printf("\n");
    tfree();
    return 0;
}

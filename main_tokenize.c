#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "value.h"
#include "tokenizer.h"
#include "talloc.h"
#include "linkedlist.h"

int main(void) {
   Value *list = tokenize(stdin);
   displayTokens(list);
   //display(list);
   tfree();
   return 0;
}

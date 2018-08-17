#include <stdio.h>
#include <string.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"

int main(void) {
   Value *val1 = talloc(sizeof(Value));
   val1->type = INT_TYPE;
   val1->i = 23;

   Value *val2 = talloc(sizeof(Value));
   val2->type = STR_TYPE;
   val2->s = talloc(10 * sizeof(char));
   strcpy(val2->s, "tofu");

   Value *head = makeNull();
   head = cons(val1, head);
   head = cons(val2, head);
   display(head);
   Value *head2;
   head2 = reverse(head);
   display(head2);
   tfree();

   head = makeNull();
   for (int i = 0; i < 20; i++) {
     val1 = talloc(sizeof(Value));
     val1->type = INT_TYPE;
     val1->i = i%10;
     head = cons(val1, head);
   }
   display(head);

   tfree();
   printf("I can see this.\n");
   Value *val3 = talloc(sizeof(Value));
   texit(0);
   printf("I should never see this.\n");
   return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include "value.h"
#include "linkedlist.h"

int main(void) {
   Value *val1 = malloc(sizeof(Value));
   val1->type = INT_TYPE;
   val1->i = 23;

   Value *val2 = malloc(sizeof(Value));
   val2->type = STR_TYPE;
   val2->s = malloc(5 * sizeof(char));
   strcpy(val2->s, "tofu");

   Value *head = makeNull();
   printf("Empty? %i\n", isNull(head));
   head = cons(val1, head);
   head = cons(val2, head);

   printf("List: ");
   display(head);
   Value *headcar = car(head);
   printf("Car of list: %s\n", headcar -> s);
   Value *headcdr = cdr(head);
   printf("Cdr of list: ");
   display(headcdr);
   printf("Length = %i\n", length(head));
   printf("Empty? %i\n", isNull(head));
   Value *headreversed = reverse(head);
   printf("List reversed: ");
   display(headreversed);
   //cleanup(head);
   //cleanup(headreversed);

   val2 = malloc(sizeof(Value));
   val2->type = STR_TYPE;
   val2->s = malloc(2*sizeof(char));
   strcpy(val2->s, ")");
   Value *val3 = malloc(sizeof(Value));
   val3->type = INT_TYPE;
   val3->i = 17;
   Value *val4 = malloc(sizeof(Value));
   val4->type = STR_TYPE;
   val4->s = malloc(2*sizeof(char));
   strcpy(val4->s, "x");
   Value *val5 = malloc(sizeof(Value));
   val5->type = STR_TYPE;
   val5->s = malloc(2*sizeof(char));
   strcpy(val5->s, "+");
   Value *val6 = malloc(sizeof(Value));
   val6->type = STR_TYPE;
   val6->s = malloc(2*sizeof(char));
   strcpy(val6->s, "(");
   Value *val7 = malloc(sizeof(Value));
   val7->type = STR_TYPE;
   val7->s = malloc(4*sizeof(char));
   strcpy(val7->s, "(x)");
   Value *val8 = malloc(sizeof(Value));
   val8->type = STR_TYPE;
   val8->s = malloc(7*sizeof(char));
   strcpy(val8->s, "lambda");
   head = makeNull();
   head = cons(val2, head);
   head = cons(val3, head);
   head = cons(val4, head);
   head = cons(val5, head);
   head = cons(val6, head);
   head = cons(val7, head);
   head = cons(val8, head);
   printf("List 2: ");
   display(head);
   headreversed = reverse(head);
   printf("List 2 reversed: ");
   display(headreversed);

  // cleanup(head);
  // cleanup(headreversed);

   return 0;
}

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

/*
 * Create an empty list (a new Value object of type NULL_TYPE).
*/
Value *makeNull() {
   Value *lst = talloc(sizeof(Value));
   lst->type = NULL_TYPE;
   return lst;
}

/*
 * Create a nonempty list (a new Value object of type CONS_TYPE).
 */
Value *cons(Value *car, Value *cdr) {
   Value *lst = talloc(sizeof(Value));
   lst->type = CONS_TYPE;
   (lst->c).car = car;
   (lst->c).cdr = cdr;
   return lst;
}

/*
 * Print a representation of the contents of a linked list.
 */
void display(Value *list) {
   printf("( ");
   Value *next = list;
   assert(next);
   while ((next->type) != NULL_TYPE) {
      Value *val = (next->c).car;
      assert(val);
      switch (val->type) {
         case INT_TYPE:
            printf("%i ", val->i);
            break;
         case DOUBLE_TYPE:
            printf("%f ", val->d);
            break;
         case SYMBOL_TYPE:
         case STR_TYPE: // go to bool case because both are strings
         case BOOL_TYPE:
            printf("%s ", val->s);
            break;
         case PTR_TYPE:
            printf("%p ", val->p);
            break;
         case OPEN_TYPE:
            printf("( ");
            break;
         case CLOSE_TYPE:
            printf(") ");
            break;
         case CONS_TYPE:
              display(val);
              break;
         default:
            printf("Error! Why is there null cell in the middle?\n");
      }
      next = cdr(next);
      assert(next);
   }
   printf(")\n");
   return;
}

/*
 * Get the car value of a given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
*/
Value *car(Value *list) {
   assert(list);
   assert((list->type) == CONS_TYPE);
   return (list->c).car;
}

/*
 * Get the cdr value of a given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
Value *cdr(Value *list) {
   assert(list);
   assert((list->type) == CONS_TYPE);
   return (list->c).cdr;
}

/*
 * Test if the given value is a NULL_TYPE value.
 * Uses assertions to ensure that this is a legitimate operation.
 */
bool isNull(Value *value) {
   assert(value);
   assert((value->type) == CONS_TYPE || (value->type) == NULL_TYPE);
   return (value->type) == NULL_TYPE;
}

/*
 * Compute the length of the given list.
 * Uses assertions to ensure that this is a legitimate operation.
*/
int length(Value *value) {
   assert(value);
   assert((value->type) == CONS_TYPE || (value->type) == NULL_TYPE);
   int len = 0;
   Value *next = value;
   while ((next->type) != NULL_TYPE) {
      len++;
      next = cdr(next);
      assert(next);
   }
   return len;
}

/*
 * Create a new linked list whose entries correspond to the given list's
 * entries, but in reverse order.  The resulting list is a shallow copy of the
 * original: that is, stored data within the linked list should NOT be
 * duplicated; rather, the new list's (new) CONS_TYPE nodes should point to
 * precisely the items in the original list.
 *
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
Value *reverse(Value *list) {
   assert(list);
   assert((list->type) == CONS_TYPE || (list->type) == NULL_TYPE);
   Value *newlst = makeNull();
   Value *next = list;
   while ((next->type) != NULL_TYPE) {
      newlst = cons((next->c).car, newlst);
      next = cdr(next);
      assert(next);
   }
   return newlst;
}

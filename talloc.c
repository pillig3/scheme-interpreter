#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include <assert.h>

static Value *head;


/*
 * Create an empty list (a new Value object of type NULL_TYPE).
*/
Value *makeNullT() {
   Value *lst = malloc(sizeof(Value));
   assert(lst);
   lst->type = NULL_TYPE;
   return lst;
}

/*
 * Create a nonempty list (a new Value object of type CONS_TYPE).
 */
Value *consT(Value *car, Value *cdr) {
   Value *lst = malloc(sizeof(Value));
   assert(lst);
   lst->type = CONS_TYPE;
   (lst->c).car = car;
   (lst->c).cdr = cdr;
   return lst;
}

/*
 * Get the car value of a given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
*/
Value *carT(Value *list) {
   assert((list->type) == CONS_TYPE);
   return (list->c).car;
}

/*
 * Get the cdr value of a given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
Value *cdrT(Value *list) {
   assert((list->type) == CONS_TYPE);
   return (list->c).cdr;
}



/*
 * A malloc-like function that allocates memory, tracking all allocated
 * pointers in the "active list."  (You can choose your implementation of the
 * active list, but whatever it is, your talloc code should NOT call functions
 * in linkedlist.h; instead, implement any list-like behavior directly here.
 * Otherwise you'll end up with circular dependencies, since you're going to
 * modify the linked list to use talloc instead of malloc.)
 */
void *talloc(size_t size) {
   void *pointer = malloc(size);
   assert(pointer);
   Value *toAdd = makeNullT();
   toAdd -> type = PTR_TYPE;
   toAdd -> p = pointer;
   if(!head) {
      head = makeNullT();
   }
   head = consT(toAdd,head);
   return pointer;
}

/*
 * Free all pointers allocated by talloc, as well as whatever memory you
 * malloc'ed to create/update the active list.
 */
void tfree() {
   assert(head);
   Value *next = head;
   while (next && next->type != NULL_TYPE) {
     Value *val = carT(next);
     assert(val->type == PTR_TYPE);
     free(val->p);
     free(val);
     Value *temp = cdrT(next);
     free(next);
     next = temp;
   }
   free(next);
   // Makes head null so a new list will be created next time talloc is called
   head = NULL;
}

/*
 * A simple two-line function to stand in the C function "exit", which calls
 * tfree() and then exit().  (You'll use this later to allow a clean exit from
 * your interpreter when you encounter an error: so memory can be automatically
 * cleaned up when exiting.)
 */
void texit(int status) {
   tfree();
   exit(status);
}

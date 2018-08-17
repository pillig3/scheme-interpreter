#include <stdlib.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define UNDEFINED_SYMBOL "23" // Not a symbol in Scheme - so if we run into
                              // this as a symbol, we know to throw an error

/*
* Prints out a supplied error message and terminates the program.
*/
void evaluationError(char *msg) {
    printf("Evaluation Error: %s\n", msg);
    texit(1);
}

/*
* Prints a single value
*/
void printValue(Value *val) {
    switch (val->type) {
        case BOOL_TYPE:
            printf("%s", val->b ? "#t" : "#f");
            break;
        case STR_TYPE:
            printf("\"");
            printf("%s", val->s);
            printf("\"");
            break;
        case SYMBOL_TYPE:
            printf("%s", val->s);
            break;
        case INT_TYPE:
            printf("%i", val->i);
            break;
        case DOUBLE_TYPE:
            printf("%f", val->d);
            break;
        case CONS_TYPE:
            if (cdr(val)->type == NULL_TYPE) { // one thing in list
                printf("(");
                printValue(car(val));
                printf(")");
            } else if (cdr(val)->type != CONS_TYPE) {
                printf("(");
                printValue(car(val));
                printf(" . ");
                printValue(cdr(val));
                printf(")");
            }
            else {
                printf("(");
                while (val->type == CONS_TYPE) {
                    if (cdr(val)->type != CONS_TYPE &&
                        cdr(val)->type != NULL_TYPE) {
                        printValue(car(val));
                        val = cdr(val);
                        printf(" . ");
                    } else {
                        printValue(car(val));
                        val = cdr(val);
                        if (val->type != NULL_TYPE) {
                            printf(" ");
                        }
                    }
                }
                if (val->type != NULL_TYPE) {
                    printValue(val);
                }
                printf(")");
            }
            break;
        case CLOSURE_TYPE:
            printf("#<procedure>");
            break;
        case NULL_TYPE:
            printf("()");
            break;
        case DOT_TYPE:
            printf(".");
            break;
        default:
          break;
    }
}


/********************************************************************
*********************************************************************
***** FUNCTIONS: apply and bind                                 *****
*****                                                           *****
***** Allows us to bind a function name to the associated       *****
***** procedure and to evaluate said function given a list      *****
***** of args.                                                  *****
*********************************************************************
********************************************************************/


/*
* Allows for the implementation of variadic lambda functions. Checks that
* there is exactly one parameter after "." and bindings this parameter to
* the list of the remaining args.
*/
Value *addVariadicBinding(Value *parameters, Value *args, Value *bindings) {
    Value *curBind = cons(args, makeNull());
    assert(car(parameters)->type == DOT_TYPE);
    parameters = cdr(parameters);  // delete '.'
    if (parameters->type == NULL_TYPE || cdr(parameters)->type != NULL_TYPE) {
        evaluationError("Wrong number of args after . in parameters list");
    }
    curBind = cons(car(parameters), curBind);
    bindings = cons(curBind, bindings);
    return bindings;
}

/*
* Applies the given function to the given arguments.
*/
Value *apply(Value *function, Value *args) {
    Frame *newFrame = talloc(sizeof(Frame));
    if (!(function->type == CLOSURE_TYPE ||
          function->type == PRIMITIVE_TYPE)) {
        evaluationError("function should be closure or primitive type");
    }
    if (function->type == PRIMITIVE_TYPE) {
        return (function->pf)(args);
    }
    newFrame->parent = (function->k).frame;
    Value *bindings = makeNull();
    Value *newValue; // e.g., newValue = v1
    Value *curBind;
    Value *parameters = (function->k).parameters;
    if (parameters->type == SYMBOL_TYPE) { // variadic
        curBind = makeNull();
        curBind = cons(args, curBind);
        curBind = cons(parameters, curBind);
        bindings = cons(curBind, bindings);
    } else { // n-adic, must bind multiple things
        // args is null and parameters aren't
        if (args->type == NULL_TYPE && parameters->type != NULL_TYPE) {
            evaluationError("Not enough parameters in function call.");
        } else if (parameters->type == NULL_TYPE && args->type != NULL_TYPE) {
            evaluationError("Too many parameters in function call.");
        } else if (parameters->type == NULL_TYPE && args->type == NULL_TYPE) {

        } else {
            while (args->type != NULL_TYPE) {
                if (parameters->type == NULL_TYPE) {
                    evaluationError("Too many parameters in function call.");
                }
                // variadic case: handle "." as a parameter
                else if (car(parameters)->type == DOT_TYPE) {
                    bindings = addVariadicBinding(parameters, args, bindings);
                    parameters = makeNull();
                    args = makeNull();
                } else {
                    newValue =  car(args);
                    curBind = makeNull();
                    curBind = cons(newValue, curBind);
                    curBind = cons(car(parameters), curBind);
                    bindings = cons(curBind, bindings);
                    args = cdr(args);
                    parameters = cdr(parameters);
                }
            }
            if (parameters->type != NULL_TYPE) {
                if (car(parameters)->type == DOT_TYPE) {
                    bindings = addVariadicBinding(parameters, args, bindings);
                    parameters = makeNull();
                    args = makeNull();
                } else {
                    evaluationError("Not enough parameters in fx call.");
                }
            }
        }
    }
    newFrame->bindings = bindings;
    return eval((function->k).function, newFrame);
}

/*
* Binds the string 'name' to the function in the given frame
*/
void bind(char *name, Value *(*function)(Value *), Frame *frame) {
    Value *value = makeNull();
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *newBinding = cons(value, makeNull());
    Value *nameVal = makeNull();
    nameVal->type = SYMBOL_TYPE;
    nameVal->s = name;
    newBinding = cons(nameVal, newBinding);
    frame->bindings = cons(newBinding, frame->bindings);
}


/********************************************************************
*********************************************************************
***** Primitive Functions:                                      *****
*****     add, multiply, subtract, divide                       *****
*****     null?, car, cdr, cons                                 *****
*****     <=, eq?, apply, error                                 *****
*****     pair?, number?                                        *****
*****                                                           *****
***** And associated helper functions                           *****
*********************************************************************
********************************************************************/


/*
* Given a list of values, returns their sum if all values are numbers.
* Otherwise, throws an evaluation error.
*/
Value *primitiveAdd(Value *args) {
    if (! (args->type == CONS_TYPE || args->type == NULL_TYPE)) {
        evaluationError("Wrong argument type provided for +");
    }
    int sum = 0;
    double dSum = 0.0;
    bool isDouble = false;
    while (args->type == CONS_TYPE) {
        Value *cur = car(args);
        if (! (cur->type == INT_TYPE || cur->type == DOUBLE_TYPE)) {
            evaluationError("Wrong argument type provided for +");
        }
        if (isDouble) { // current sum is a double
            if (cur->type == INT_TYPE) {
                dSum += cur->i;
            } else {
                dSum += cur->d;
            }
        } else if (cur->type == INT_TYPE) { // keep current sum as an int
            sum += cur->i;
        } else { // current sum is an int but needs to change to a double
            dSum = sum;
            dSum += cur->d;
            isDouble = true;
        }
        args = cdr(args);
    }
    Value *newVal = makeNull();
    if (isDouble) {
        newVal->type = DOUBLE_TYPE;
        newVal->d = dSum;
        return newVal;
    }
    newVal->type = INT_TYPE;
    newVal->i = sum;
    return newVal;
}

/*
* Given a list of values, returns their product if all values are numbers.
* Otherwise, throws an evaluation error.
*/
Value *primitiveMultiply(Value *args) {
    if (! (args->type == CONS_TYPE || args->type == NULL_TYPE)) {
        evaluationError("Wrong argument type provided for *");
    }
    int iProd = 1;
    double dProd = 1.0;
    bool isDouble = false;
    while (args->type == CONS_TYPE) {
        Value *cur = car(args);
        if (! (cur->type == INT_TYPE || cur->type == DOUBLE_TYPE)) {
            evaluationError("Wrong argument type provided for *");
        }
        if (isDouble) { // current product is a double
            if (cur->type == INT_TYPE) {
                dProd *= cur->i;
            } else {
                dProd *= cur->d;
            }
        } else if (cur->type == INT_TYPE) { // keep current product as int
            iProd *= cur->i;
        } else { // current product is int but needs to change to double
            dProd = iProd;
            dProd *= cur->d;
            isDouble = true;
        }
        args = cdr(args);
    }
    Value *newVal = makeNull();
    if (isDouble) {
        newVal->type = DOUBLE_TYPE;
        newVal->d = dProd;
        return newVal;
    }
    newVal->type = INT_TYPE;
    newVal->i = iProd;
    return newVal;
}

/*
* Given a list of numbers, subtracts every following number from
* the first number
*/
Value *primitiveSubtract(Value *args) {
    // make sure there are at least two arguments
    if (args->type != CONS_TYPE) {
        evaluationError("Wrong number of arguments provided for -");
    }
    int iResult;
    double dResult;
    bool isDouble = false;
    if (car(args)->type == INT_TYPE) {
        iResult = car(args)->i;
    } else if (car(args)->type == DOUBLE_TYPE) {
        dResult = car(args)->d;
        isDouble = true;
    } else {
        evaluationError("Wrong argument type provided for -");
    }
    args = cdr(args);
    if (args->type == NULL_TYPE) { // single arg
        if (isDouble) {
            dResult = dResult * -1;
        } else {
            iResult = iResult * -1;
        }
    }
    while (args->type == CONS_TYPE) {
        Value *cur = car(args);
        if (! (cur->type == INT_TYPE || cur->type == DOUBLE_TYPE)) {
            evaluationError("Wrong argument type provided for -");
        }
        if (isDouble) { // current result is a double
            if (cur->type == INT_TYPE) {
                dResult -= cur->i;
            } else {
                dResult -= cur->d;
            }
        } else if (cur->type == INT_TYPE) { // current result is an int
                iResult -= cur->i;
        } else { // current result is an int and needs to change to a double
            dResult = iResult;
            dResult -= cur->d;
            isDouble = true;
        }
        args = cdr(args);
    }
    Value *newVal = makeNull();
    if (isDouble) {
        newVal->type = DOUBLE_TYPE;
        newVal->d = dResult;
        return newVal;
    }
    newVal->type = INT_TYPE;
    newVal->i = iResult;
    return newVal;
}

/*
* Given a list of numbers, divides the first number by each
* successive number.
*/
Value *primitiveDivide(Value *args) {
    // make sure there is at least one argument
    if (args->type != CONS_TYPE) {
        evaluationError("Wrong number of arguments provided for /");
    }
    int iResult;
    double dResult;
    bool isDouble = false;
    if (car(args)->type == INT_TYPE) {
        iResult = car(args)->i;
    } else if (car(args)->type == DOUBLE_TYPE) {
        dResult = car(args)->d;
        isDouble = true;
    } else {
        evaluationError("Wrong argument type provided for /");
    }
    args = cdr(args);
    if (args->type == NULL_TYPE) { // single argument
        if (!isDouble) {
            dResult = iResult;
            isDouble = true;
        }
        if (dResult == 0) {
            evaluationError("Can't divide by zero.");
        }
        dResult = 1.0 / dResult;
    }
    while (args->type == CONS_TYPE) {
        Value *cur = car(args);
        // check division by zero
        if ((cur->type == INT_TYPE && cur->i == 0) ||
            (cur->type == DOUBLE_TYPE && cur->d == 0)) {
            evaluationError("Cannot divide by zero");
        }
        if (! (cur->type == INT_TYPE || cur->type == DOUBLE_TYPE)) {
            evaluationError("Wrong argument type provided for /");
        }
        if (isDouble) { // current result is a double
            if (cur->type == INT_TYPE) {
                dResult /= cur->i;
            } else {
                dResult /= cur->d;
            }
        } else if (cur->type == INT_TYPE) {
            // if the next int divides current result evenly, result is int.
            // Otherwise, switch to double.
            if (((double)iResult / (double)cur->i) == iResult/cur->i) {
                iResult /= cur->i;
            } else {
                dResult = iResult;
                dResult /= cur->i;
                isDouble = true;
            }
        } else { // current result is int and needs to change to double
            dResult = iResult;
            dResult /= cur->d;
            isDouble = true;
        }
        args = cdr(args);
    }
    Value *newVal = makeNull();
    if (isDouble) {
        newVal->type = DOUBLE_TYPE;
        newVal->d = dResult;
        return newVal;
    }
    newVal->type = INT_TYPE;
    newVal->i = iResult;
    return newVal;
}

/*
* Helper function for <=. Returns the number stored in the
* value n, after converting it to a double (if it is not)
*/
double getNumber(Value *n) {
    assert(n->type == INT_TYPE || n->type == DOUBLE_TYPE);
    if (n->type == INT_TYPE) {
        return (double)n->i;
    }
    return n->d;
}

/*
* Given numbers n1, n2, ..., nk, returns true if
* n1 ≤ n2  ≤ ... ≤ nk, otherwise returns false.
*/
Value *primitiveLeq(Value *args) {
    // make sure there are at least two arguments
    if (args->type == NULL_TYPE || cdr(args)->type == NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for <=");
    }
    if (car(args)->type != INT_TYPE && car(args)->type != DOUBLE_TYPE) {
        evaluationError("Wrong argument type provided for <=");
    }
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    double current = getNumber(car(args));
    args = cdr(args);
    while (args->type != NULL_TYPE) {
        if (car(args)->type != INT_TYPE && car(args)->type != DOUBLE_TYPE) {
            evaluationError("Wrong argument type provided for <=");
        }
        if (current > (double)getNumber(car(args))) {
            result->b = false;
            return result;
        }
        current = getNumber(car(args));
        args = cdr(args);
    }
    result->b = true;
    return result;
}

/*
* Reverses a scheme list
*/
Value *reverseList(Value *list) {
    Value *newList = makeNull();
    while (list->type != NULL_TYPE) {
        assert(list->type == CONS_TYPE);
        newList = cons(car(list), newList);
        list = cdr(list);
    }
    return newList;
}

/*
* Given at leat two numbers, returns true iff all numbers are equal.
* Bound to "="
*/
Value *primitiveEqSign(Value *args) {
    // make sure there are at least two arguments
    if (args->type == NULL_TYPE || cdr(args)->type == NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for =");
    }
    Value *val1 = primitiveLeq(args);
    Value *val2 = primitiveLeq(reverseList(args));
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    // true iff n1 ≤ n2  ≤ ... ≤ nk AND nk ≤ ... ≤ n2 ≤ n1
    returnVal->b = val1->b && val2->b;
    return returnVal;
}

/*
* Given two expressions, returns true if they are "the same".
* If they are different types, always returns false. For two values of
* the same type, being "the same" means different things for different types.
* See comments inside for specifics.
*/
Value *primitiveEq(Value *args) {
    // make sure there are exactly two arguments
    if (args->type == NULL_TYPE || cdr(args)->type == NULL_TYPE
        || (cdr(cdr(args)))->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for eq?");
    }
    Value *v1 = car(args);
    Value *v2 = car(cdr(args));
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    if (v1->type != v2->type) {
        returnVal->b = false;
    } else {
        switch (v1->type) {
            case INT_TYPE:
            case DOUBLE_TYPE:{
                // true if they are the same according to "="
                Value *argList = makeNull();
                argList = cons(v2, argList);
                argList = cons(v1, argList);
                returnVal = primitiveEqSign(argList);
                break;}
            case NULL_TYPE:
                // both are the empty list, return true
                returnVal->b = true;
                break;
            case SYMBOL_TYPE:
                // true if they are symbols with the same name
            case STR_TYPE:
                // true if they are the same sequence of chars
                returnVal->b = !strcmp(v1->s, v2->s);
                break;
            case BOOL_TYPE:
                // true if they are both true or both false
                returnVal->b = !(v1->b ^ v2->b);
                break;
            case PRIMITIVE_TYPE:
            case CLOSURE_TYPE:
            case CONS_TYPE:
                // true if they have the same pointer
                returnVal->b = ((int)v1 == (int)v2);
                break;
            default:
                evaluationError("Wrong argument type provided for eq?");
                break;
        }
    }
    return returnVal;
}

/*
* Given a single argument, returns true iff it is null
*/
Value *primitiveIsNull(Value *args) {
    // make sure there is exactly one argument
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for null?");
    }
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    returnVal->b = (car(args)->type == NULL_TYPE);
    return returnVal;
}

/*
* Given a number, returns true iff it is the int 0 or the double 0.0
*/
Value *primitiveZero(Value *args) {
    // make sure there is exactly one argument
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for zero?");
    }
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    if (car(args)->type == INT_TYPE) {
        returnVal->b = car(args)->i == 0;
    } else if (car(args)->type != DOUBLE_TYPE) {
        evaluationError("Wrong argument type provided for zero?");
    } else {
        // otherwise it's a double
        returnVal->b = car(args)->d == 0;
    }
    return returnVal;
}

/*
* Given a Scheme list, returns the car (the first element)
*/
Value *primitiveCar(Value *args) {
    // make sure there is exactly one argument
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for car");
    } else if (car(args)->type != CONS_TYPE) {
        evaluationError("Wrong argument type provided for car");
    }
    return car(car(args));
}

/*
* Given a Scheme list, returns the cdr (the list without the first element)
*/
Value *primitiveCdr(Value *args) {
    // make sure there is exactly one argument
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for cdr");
    } else if (car(args)->type != CONS_TYPE) {
        evaluationError("Wrong argument type provided for cdr");
    }
    Value *currArg = car(args);
    if (cdr(currArg)->type == CONS_TYPE &&
        car(cdr(currArg))->type == DOT_TYPE) {
        // need to remove . in list
        Value *newArgs = cdr(cdr(currArg));
        if (newArgs->type != CONS_TYPE || cdr(newArgs)->type != NULL_TYPE) {
            evaluationError("Wrong number of arguments after . in list");
        }
        return car(newArgs);
    }
    return cdr(car(args));
}

/*
* Given a1, a2, returns the dotted pair (a1 . a2)
*/
Value *primitiveCons(Value *args) {
    // make sure there are exactly two arguments
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE ||
       cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for cons");
    }
    return cons(car(args), car(cdr(args)));
}

/*
* Scheme function to throw an error, printing any given message
*/
Value *primitiveError(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError("Error thrown by (error)");
    } else if (args->type == CONS_TYPE && car(args)->type == STR_TYPE
                && cdr(args)->type == NULL_TYPE) {
        evaluationError(car(args)->s);
    } else {
        evaluationError("Invalid syntax in error");
    }
    Value *returnVal = makeNull();
    returnVal->type = VOID_TYPE;
    return returnVal;
}

/*
* Check if given argument is a pair, return true if so.
*/
Value *primitivePair(Value *args) {
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for pair? ");
    }
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->b = (car(args)->type == CONS_TYPE);
    return result;
}

/*
* Given a list of args, check that there is exactly one argument provided.
* Check if that argument is a list, if so, return true via a Value*, else
* return false.
*/
Value *primitiveList(Value *args) {
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Provided the wrong number of arguments for list? ");
    }
    args = car(args);
    while (args->type == CONS_TYPE) {
        args = cdr(args);
    }
    if (args->type == NULL_TYPE) {
        result->b = true;
    } else {
        result->b = false;
    }
    return result;
}

/*
* Given a list of arguments, concatenates the "cdr" into a single list
* (while verifying that the final item is a proper list) and calls the
* C version of apply with the car as the procedure and the remaining items
* as args.
*/
Value *primitiveApply(Value *args) {
    Value *result;
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE) {
        evaluationError("Wrong number of arguments for primitive apply.");
    }
    Value *proc = car(args);
    args = cdr(args);
    Value *newArgs = makeNull();
    while (args->type == CONS_TYPE) {
        newArgs = cons(car(args), newArgs);
        args = cdr(args);
    }
    Value *primitiveFirst = primitiveList(cons(car(newArgs), makeNull()));
    if (primitiveFirst->b) {
        assert(newArgs->type == CONS_TYPE);
        Value *newList = car(newArgs);
        newArgs = cdr(newArgs);
        while (newArgs->type != NULL_TYPE) {
            newList = cons(car(newArgs), newList);
            newArgs = cdr(newArgs);
        }
        result = apply(proc, newList);
    } else {
        evaluationError("Last argument for primitive apply is not a list");
    }
    return result;
}

/*
* Returns true if the given Value is a number
*/
Value *primitiveNumber(Value *args) {
    if (args->type == NULL_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for number?");
    }
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    returnVal->b = (car(args)->type == INT_TYPE ||
                    car(args)->type == DOUBLE_TYPE);
    return returnVal;
}

/*********************************************************************
**********************************************************************
***** makeFrame, interpret and lookUpSymbol                      *****
*****                                                            *****
***** makeFrame and interpret help with initialize of program    *****
***** by creating a null frame and binding the primitive         *****
***** procedures in that frame. Interpret also iterates through  *****
***** S-expressions, calling eval on each. lookUpSymbol allows   *****
***** for access to the frame's bindings throughout the program. *****
**********************************************************************
*********************************************************************/

/*
* Makes and returns a frame with NULL parent
*/
Frame *makeFrame() {
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = NULL;
    Value *bindings = makeNull();
    frame->bindings = bindings;
    return frame;
}

/*
* Given a list of S-expressions (i.e., the output of parser), calls eval on
* each S-expression in the top-level environment. Prints the result of each
* eval.
*/
void interpret(Value *tree, Frame *frame) {
    // Add bindings for primitive functions
    bind("+", primitiveAdd, frame);
    bind("null?", primitiveIsNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
    bind("*", primitiveMultiply, frame);
    bind("-", primitiveSubtract, frame);
    bind("<=", primitiveLeq, frame);
    bind("/", primitiveDivide, frame);
    bind("eq?", primitiveEq, frame);
    bind("apply", primitiveApply, frame);
    bind("error", primitiveError, frame);
    bind("pair?", primitivePair, frame);
    bind("number?", primitiveNumber, frame);

    Value *cur;
    while (tree->type == CONS_TYPE) {
        cur = car(tree);
        Value *val = eval(cur, frame);
        printValue(val);
        if (val->type != VOID_TYPE) {
            printf("\n");
        }
        tree = cdr(tree);
    }
}

/*
* Given a value of symbol type and a frame, looks up the binding of that
* value in the given environment
*/
Value *lookUpSymbol(Value *symbol, Frame *frame) {
    Value *cur;
    Value *bindings = frame->bindings;
    while (bindings->type == CONS_TYPE) {
        cur = car(bindings);
        assert(cur->type == CONS_TYPE);
        Value *curSymbol = car(cur);
        assert(curSymbol->type == SYMBOL_TYPE);
        if (!strcmp(curSymbol->s, symbol->s)) {
            return car(cdr(cur));
        }
        bindings = cdr(bindings);
    }
    if (!frame->parent) {
        char *msg = talloc(sizeof(symbol->s)*sizeof(char)+28);
        strcpy(msg, "Failed to find the symbol: ");
        strcat(msg, symbol->s);
        evaluationError(msg);
    }
    return lookUpSymbol(symbol, frame->parent);
}

/*********************************************************************
**********************************************************************
***** Special Forms                                              *****
*****     let, letrec, let*                                      *****
*****     and, or, cond                                          *****
*****     define, lambda, set!, begin                            *****
*****     load                                                   *****
*****                                                            *****
***** Evaluate these special forms, as well as any other valid   *****
***** input through the various functions in this section, as    *****
***** directed via eval.                                         *****
**********************************************************************
*********************************************************************/

/*
* Given a proposed new binding within a let, let*, or letrec statement, checks
* validity by throwing an error if the syntax is incorrect.
*/
void assertValidSyntax(Value *newBinding) {
    if (newBinding->type != CONS_TYPE || cdr(newBinding)->type != CONS_TYPE) {
        evaluationError("Missing block in let assignment.");
    } else if (cdr(cdr(newBinding))->type != NULL_TYPE) {
        evaluationError("Too many blocks provided in let assignment.");
    } else if (car(newBinding)->type != SYMBOL_TYPE) {
        evaluationError("Let can only bind to a symbol.");
    }
}

/*
* Given a proposed new binding within a let or letrec statement and a list of
* bindings that are getting added to the same frame, checks validity
* by throwing an error if the variable is already bound within the new frame.
* (in let and letrec, a variable can only appear once in the bindings).
*/
void assertValidLetSyntax(Value *newBinding, Value *frameBindings) {
    Value *cur;
    while (frameBindings->type == CONS_TYPE) {
        cur = car(frameBindings);
        if (!strcmp(car(newBinding)->s, car(cur)->s)) {
            evaluationError("Duplicate identifier in let assignment.");
        }
        frameBindings = cdr(frameBindings);
    }
}

/*
* Returns the first false value, if none are false returns the last value.
* If there are no values given, returns #t
*/
Value *evalAnd(Value *args, Frame *frame) {
    Value *result = makeNull();
    if (args->type == NULL_TYPE) {
        result->type = BOOL_TYPE;
        result->b = true;
    }
    while (args->type == CONS_TYPE) {
        result = eval(car(args), frame);
        if (result->type == BOOL_TYPE && !result->b) {
            return result;
        }
        args = cdr(args);
    }
    return result;
}

/*
* Returns the first true (not "#f") value, if none are true returns the last
* value. If there are no values given, returns #f
*/
Value *evalOr(Value *args, Frame *frame) {
    Value *result = makeNull();
    if (args->type == NULL_TYPE) {
        result->type = BOOL_TYPE;
        result->b = false;
    }
    while (args->type == CONS_TYPE) {
        result = eval(car(args), frame);
        if ((result->type != BOOL_TYPE || result->b)) {
            return result;
        }
        args = cdr(args);
    }
    return result;
}

/*
* Given a list of either 2 or 3 arguments, applies the "if" operator:
* evaluates the first argument, and if not false, returns the evaluated
* value of the second argument. Otherwise, returns the evaluated value
* of the third argument, if one exists.
*/
Value *evalIf(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE) {
        evaluationError("Not enough blocks in an if statement");
    }
    if (cdr(cdr(args))->type != NULL_TYPE &&
        cdr(cdr(cdr(args)))->type != NULL_TYPE) {
        evaluationError("Too many blocks in an if statement");
    }
    Value *result = eval(car(args), frame);
    if (!(result->type == BOOL_TYPE) || !(result->b==false)) {
        return eval(car(cdr(args)), frame);
    } else if (cdr(cdr(args))->type != NULL_TYPE) {
        return eval(car(cdr(cdr(args))), frame);
    }
    Value *voidVal = makeNull();
    voidVal->type = VOID_TYPE;
    return voidVal;
}

/*
* Helper function for evalCond: throws an error if the clause list in cond
* is not of the correct form.
*/
void assertValidClauseList(Value *args) {
    Value *clauseList = args;
    while (clauseList->type != NULL_TYPE) {
        Value *curClause = car(clauseList);
        clauseList = cdr(clauseList);
        // check that it is a list of two items
        if (curClause->type != CONS_TYPE || cdr(curClause)->type != CONS_TYPE
            || cdr(cdr(curClause))->type != NULL_TYPE) {
            evaluationError("Wrong number of items in a cond clause");
        }
        // check that if it is an "else" clause, it is the last one
        if (car(curClause)->type == SYMBOL_TYPE &&
            !strcmp(car(curClause)->s, "else") &&
            clauseList->type != NULL_TYPE) {
            evaluationError("'else' can only appear as the last clause "
                            "in cond statement");
        }
    }
}

/*
* Evaluates (cond list-of-clauses), where list-of-clauses has the form
* (test1 expr1) ... (testn exprn) or (test1 expr1) ... (else exprn).
* Evaluates the tests in order, until one evaluates to true, then evaluates
* and returns the expr corresponding to that test. If all tests are false and
* there is no else, returns a void type Value.
*/
Value *evalCond(Value *args, Frame *frame) {
    assertValidClauseList(args);
    Value *returnVal = makeNull();
    returnVal->type = VOID_TYPE;
    Value *result;
    bool done = false;
    while (args->type != NULL_TYPE && !done) {
        Value *curClause = car(args);
        args = cdr(args);
        if (car(curClause)->type == SYMBOL_TYPE &&
            !strcmp(car(curClause)->s, "else")) {
            result = makeNull();
        } else {
            result = eval(car(curClause), frame);
        }
        if (result->type != BOOL_TYPE || result->b == true) {
            returnVal = eval(car(cdr(curClause)), frame);
            done = true;
        }
    }
    return returnVal;
}

/*
* Creates a binding of (x1, e1) within a let or let* statement
*/
Value *createBinding(Frame *frame, Value *bindings, Value *toBind) {
    assert(toBind->type == CONS_TYPE);
    assert(cdr(toBind)->type == CONS_TYPE);
    Value *result = eval(car(cdr(toBind)), frame);
    Value *eCurrBind = cons(result, makeNull());
    eCurrBind = cons(car(toBind), eCurrBind);
    return cons(eCurrBind, bindings);
}

/*
* Evaluates (let ((x1 e1) ... (xn en)) body1 ... bodym)
* Creates a new frame F with given parent frame, evaluates each ei in the
* parent frame to get vi and binds this to xi in F.
* Then evaluates body1,...,bodym in frame F and returns the value of bodym
*/
Value *evalLet(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE) {
        evaluationError("Not enough blocks after 'let'");
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    Value *bindings = makeNull();
    Value *toBind = car(args); // e.g., toBind = ((x1 v1) (x2 v2))
    Value *currBind; // e.g., currBind = (x1 v1)
    while (toBind->type != NULL_TYPE) {
        if (toBind->type != CONS_TYPE) {
            evaluationError("Invalid synax in 'let'");
        }
        currBind = car(toBind);
        assertValidSyntax(currBind);
        assertValidLetSyntax(currBind, bindings);
        bindings = createBinding(frame, bindings, currBind);
        toBind = cdr(toBind);
    }
    newFrame->bindings = bindings;
    // Evaluate body1, ... bodym in order, return last one.
    args = cdr(args);
    Value *returnVal = makeNull();
    while (args->type != NULL_TYPE) {
        returnVal = eval(car(args), newFrame);
        args = cdr(args);
    }
    return returnVal;
}


/*
* Evaluates (let* ((x1 e1) ... (xn en)) body1 ... bodym)
* For each i=1,...,n: Creates new frame Fi with previous frame
* as its parent, evaluates ei in frame Fi, then binds it to xi in Fi.
* Last, evaluates body1,...,bodym in frame Fn.
*/
Value *evalLetStar(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE) {
        evaluationError("Not enough blocks after 'let*'");
    }
    Frame *previousFrame = frame;
    Frame *lastFrame = talloc(sizeof(Frame)); // will be Fn
    lastFrame->parent = previousFrame;
    lastFrame->bindings = makeNull();
    Value *toBind = car(args); // e.g., toBind = ((x1 v1) (x2 v2))
    Value *currBind; // e.g., currBind = (x1 v1)
    while (toBind->type != NULL_TYPE) {
        Frame *newFrame = talloc(sizeof(Frame));
        newFrame->parent = previousFrame;
        newFrame->bindings = makeNull();
        if (toBind->type != CONS_TYPE) {
            evaluationError("Invalid synax in 'let*'");
        }
        currBind = car(toBind);
        assertValidSyntax(currBind);
        newFrame->bindings = createBinding(newFrame,
                                           newFrame->bindings, currBind);
        toBind = cdr(toBind);
        previousFrame = newFrame;
        lastFrame = newFrame;
    }
    // Evaluate body1, ... bodym in order, return last one.
    args = cdr(args);
    Value *returnVal = makeNull();
    while (args->type != NULL_TYPE) {
        returnVal = eval(car(args), lastFrame);
        args = cdr(args);
    }
    return returnVal;
}

/*
* Evaluates (letrec ((x1 e1) ... (xn en)) body1 ... bodym)
* Creates new frame F with given parent, binds each xi to an undefined
* value in F, then evaluates each ei in F, then bind the results
* to the corresponding xi's. Last, evaluates body1,...,bodym in frame F.
*/
Value *evalLetRec(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE) {
        evaluationError("Not enough blocks after 'letrec'");
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    Value *toBind = car(args); // e.g., toBind = ((x1 v1) (x2 v2))
    Value *currBind; // e.g., currBind = (x1 v1)
    // bind each xi to an undefined value
    while (toBind->type != NULL_TYPE) {
        if (toBind->type != CONS_TYPE) {
            evaluationError("Invalid synax in 'letrec'");
        }
        assertValidSyntax(car(toBind));
        Value *undefined = makeNull();
        undefined->type = SYMBOL_TYPE;
        undefined->s = UNDEFINED_SYMBOL;
        currBind = cons(undefined, makeNull());
        currBind = cons(car(car(toBind)), currBind);
        newFrame->bindings = cons(currBind, newFrame->bindings);
        toBind = cdr(toBind);
    }
    toBind = car(args);
    Value *bindings = makeNull();
    Value *eCurrBind = makeNull();
    // evaluate the ei in the new frame, add to 'bindings'.
    while (toBind->type != NULL_TYPE) {
        currBind = car(toBind);
        assertValidLetSyntax(currBind, bindings);
        Value *result = eval(car(cdr(currBind)), newFrame);
        if (result->type == SYMBOL_TYPE &&
            !strcmp(result->s, UNDEFINED_SYMBOL)) {
            evaluationError("Expression in letrec binding could not be "
                            "evaluated without assigning or referring to the "
                            "value of another variable in same letrec");
        }
        eCurrBind = cons(result, makeNull()); // e.g., (x1 eval(v1))
        eCurrBind = cons(car(currBind), eCurrBind);
        bindings = cons(eCurrBind, bindings);
        toBind = cdr(toBind);
    }
    newFrame->bindings = bindings;
    // Evaluate body1, ... bodym in order, return last one.
    args = cdr(args);
    Value *returnVal = makeNull();
    while (args->type != NULL_TYPE) {
        returnVal = eval(car(args), newFrame);
        args = cdr(args);
    }
    return returnVal;
}

/*
* Given args=(symbol, s-expr), evaluate s-expr and bind
* the result to symbol in the current frame.
*/
Value *evalDefine(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE ||
       cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for define.");
    }
    if (car(args)->type != SYMBOL_TYPE) {
        evaluationError("Define can only bind to a symbol.");
    }
    Value *result = eval(car(cdr(args)), frame);
    bool found = false;
    Value *cur; // current binding: (symbol value)
    Value *bindings = frame->bindings; // ((e1 v1) ... (en vn))
    while (bindings->type == CONS_TYPE && !found) {
        cur = car(bindings);
        assert(cur->type == CONS_TYPE);
        Value *curSymbol = car(cur);
        assert(curSymbol->type == SYMBOL_TYPE);
        if (!strcmp(curSymbol->s, car(args)->s)) {
            found = true;
            // rebind existing variable
            *cdr(cur) = *cons(result, makeNull());
        }
        bindings = cdr(bindings);
    }
    if (!found) {
        Value *newBinding = makeNull();
        newBinding = cons(result, newBinding);
        newBinding = cons(car(args), newBinding);
        frame->bindings = cons(newBinding, frame->bindings);
    }
    Value *returnValue = makeNull();
    returnValue->type = VOID_TYPE;
    return returnValue;
}

/*
* Given args=(symbol, s-expr), evaluate s-expr to get value v, then search
* frames recursively for the given symbol. If found, bind symbol to v in that
* frame. Otherwise, throw an error.
*/
Value *evalSetBang(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE ||
       cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for set!");
    }
    if (car(args)->type != SYMBOL_TYPE) {
        evaluationError("set! can only bind to a symbol");
    }
    Value *result = eval(car(cdr(args)), frame);
    Value *cur;
    Value *bindings = frame->bindings;
    while (bindings->type == CONS_TYPE) {
        cur = car(bindings);
        assert(cur->type == CONS_TYPE);
        Value *curSymbol = car(cur);
        assert(curSymbol->type == SYMBOL_TYPE);
        if (!strcmp(curSymbol->s, car(args)->s)) {
            *cdr(cur) = *cons(result, makeNull()); // redefine
            Value *returnValue = makeNull();
            returnValue->type = VOID_TYPE;
            return returnValue;
        }
        bindings = cdr(bindings);
    }
    if (!frame->parent) {
        evaluationError("Cannot set! an undefined variable");
    }
    return evalSetBang(args, frame->parent);
}

/*
* Given expressions e1, e2, ..., en, evaluate the ei in order, one at a time.
* The value of the last expression, en, is returned.
*/
Value *evalBegin(Value *args, Frame *frame) {
    Value *returnVal = makeNull();
    returnVal->type = VOID_TYPE;
    while (args->type != NULL_TYPE) {
        returnVal = eval(car(args), frame);
        args = cdr(args);
    }
    return returnVal;
}

/*
* Given ((x1 x2 ... xn) body), or (symbol body), and a frame, creates
* a closure object with parameters (x1 x2 ... xn), or variadic param
* 'symbol', function 'body', and frame 'frame'.
*/
Value *evalLambda(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE ||
        cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for lambda");
    }
    Value *closure = makeNull();
    closure->type = CLOSURE_TYPE;
    if (car(args)->type == CONS_TYPE || car(args)->type == NULL_TYPE
        || car(args)->type == SYMBOL_TYPE) {
        // parameters is either a list (if n-adic) or a symbol (if variadic)
        (closure->k).parameters = car(args);
    } else {
        evaluationError("Wrong formal parameter type in lambda definition");
    }
    (closure->k).function = car(cdr(args));
    (closure->k).frame = frame;
    return closure;
}

/*
* Loads the file with filepath given (as a value of STR_TYPE), executes the
* scheme code in the file, with given frame (printing output),
* and then returns the frame after evaluation.
*/
Frame *evalLoad(Value *args, Frame *frame) {
    // make sure there is exactly one argument
    if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE) {
        evaluationError("Wrong number of arguments provided for load");
    }
    if (car(args)->type != STR_TYPE) {
        evaluationError("Wrong argument type given for load");
    }
    FILE *file = fopen(car(args)->s, "r");
    if (!file) {
        evaluationError("The given file could not be opened");
    }
    FILE *oldStdin = stdin;
    stdin = file;
    Value *list = tokenize(false);
    Value *tree = parse(list);
    // below code modified from interpret()
    Value *cur;
    while (tree->type == CONS_TYPE) {
        cur = car(tree);
        Value *val = eval(cur, frame);
        printValue(val);
        if (val->type != VOID_TYPE) {
            printf("\n");
        }
        tree = cdr(tree);
    }
    fclose(file);
    stdin = oldStdin;
    return frame;
}

/*
* Given args=(e1 ... en), recursively evaluates e1,...,en
* to get values v1,...,vn, and returns (v1 ... vn).
*/
Value *evalCombo(Value *args, Frame *frame) {
    if (args->type == NULL_TYPE) {
        return makeNull();
    }
    return cons(eval(car(args), frame), evalCombo(cdr(args), frame));
}

/*
* Given a parse tree of a single S-expression and an environment frame,
* returns a pointer to a Value represented the expression's value.
*/
Value *eval(Value *tree, Frame *frame) {
    switch (tree->type) {
        case NULL_TYPE:
            return makeNull();
        case INT_TYPE:
        case DOUBLE_TYPE:
        case STR_TYPE:
        case BOOL_TYPE:
            return tree;
            break;
        case SYMBOL_TYPE:
            return lookUpSymbol(tree, frame);
            break;
        case CONS_TYPE: {
            Value *first = car(tree);
            Value *args = cdr(tree);
            if (first->type != SYMBOL_TYPE && first->type != CONS_TYPE) {
                evaluationError("First element in a list is not a symbol.");
            }
            if (!strcmp(first->s, "if")) {
                return evalIf(args, frame);
            } else if (!strcmp(first->s, "cond")) {
                return evalCond(args, frame);
            } else if (!strcmp(first->s, "quote")) {
                if (args->type != CONS_TYPE) {
                    evaluationError("Not enough arguments for quote.");
                } else if (cdr(args)->type != NULL_TYPE) {
                    evaluationError("Too many arguments for quote.");
                }
                return car(args);
            } else if (!strcmp(first->s, "let")) {
                return evalLet(args, frame);
            } else if (!strcmp(first->s, "and")) {
                return evalAnd(args, frame);
            } else if (!strcmp(first->s, "or")) {
                return evalOr(args, frame);
            } else if (!strcmp(first->s, "let*")) {
                return evalLetStar(args, frame);
            } else if (!strcmp(first->s, "letrec")) {
                return evalLetRec(args, frame);
            } else if (!strcmp(first->s, "define")) {
                return evalDefine(args, frame);
            } else if (!strcmp(first->s, "set!")) {
                return evalSetBang(args, frame);
            } else if (!strcmp(first->s, "begin")) {
                return evalBegin(args, frame);
            } else if (!strcmp(first->s, "lambda")) {
                return evalLambda(args, frame);
            } else if (!strcmp(first->s, "load")) {
                frame = evalLoad(args, frame);
                Value *returnVal = makeNull();
                returnVal->type = VOID_TYPE;
                return returnVal;
            } else {
                Value *results = evalCombo(cons(first, args), frame);
                return apply(car(results), cdr(results)); // Replace this
            }
            break;
        }
        default:
            evaluationError("Input not of a specified type.");
            break;
   }
   return makeNull();
}

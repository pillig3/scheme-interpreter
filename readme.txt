Note:
We have optional (and some required) functions implemented in math.scm
and lists.scm. To take advantage of this, make sure to include 
(load "lists.scm") and
(load "math.scm") at the top of each Scheme test file. 

Optional features implemented:
1. If statement can handle the form (if test consequent).
2. Can handle 'expr as a shorthand for (quote expr).
3. Let can handle the form (let ((x1 e1) ... (xn en)) body1 ... bodym),
   evaluating the bodyi in order and returning the result of evaluating bodym.
4. Correct + return type: an integer if all arguments are integers, and a
   double if at least one argument is a double.
5. The function load
6. More built-in math functions (math.scm)
7. More built-in list functions (lists.scm)
8. Tokenizes . as DOT_TYPE and correctly applies it for variadic functions
   and in quote.
9. +, *, -, and / all behave properly on 0 (for +, *) or 1 arguments.
10. REPL 

;; More built-in functions w.r.t. lists

;; Helper functions
(define =
  (lambda args
    (cond ((<= (length args) 1)
           (error "Wrong number of arguments provided for ="))
          ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
           (error "Wrong argument type provided for ="))
          (else
           (and (apply <= args) (apply <= (reverse args)))))))
(define not
  (lambda (x)
    (if x #f #t)))
(define zero?
  (lambda (x)
    (= x 0)))

; 2-operation c___rs
(define caar (lambda (lst) (car (car lst))))
(define cadr (lambda (lst) (car (cdr lst))))
(define cdar (lambda (lst) (cdr (car lst))))
(define cddr (lambda (lst) (cdr (cdr lst))))

; 3-operation c___rs
(define caaar (lambda (lst) (car (car (car lst)))))
(define caadr (lambda (lst) (car (car (cdr lst)))))
(define cadar (lambda (lst) (car (cdr (car lst)))))
(define caddr (lambda (lst) (car (cdr (cdr lst)))))
(define cdaar (lambda (lst) (cdr (car (car lst)))))
(define cdadr (lambda (lst) (cdr (car (cdr lst)))))
(define cddar (lambda (lst) (cdr (cdr (car lst)))))
(define cdddr (lambda (lst) (cdr (cdr (cdr lst)))))

; 4-operation c___rs
(define caaaar (lambda (lst) (car (car (car (car lst))))))
(define caaadr (lambda (lst) (car (car (car (cdr lst))))))
(define caadar (lambda (lst) (car (car (cdr (car lst))))))
(define caaddr (lambda (lst) (car (car (cdr (cdr lst))))))
(define cadaar (lambda (lst) (car (cdr (car (car lst))))))
(define cadadr (lambda (lst) (car (cdr (car (cdr lst))))))
(define caddar (lambda (lst) (car (cdr (cdr (car lst))))))
(define cadddr (lambda (lst) (car (cdr (cdr (cdr lst))))))
(define cdaaar (lambda (lst) (cdr (car (car (car lst))))))
(define cdaadr (lambda (lst) (cdr (car (car (cdr lst))))))
(define cdadar (lambda (lst) (cdr (car (cdr (car lst))))))
(define cdaddr (lambda (lst) (cdr (car (cdr (cdr lst))))))
(define cddaar (lambda (lst) (cdr (cdr (car (car lst))))))
(define cddadr (lambda (lst) (cdr (cdr (car (cdr lst))))))
(define cdddar (lambda (lst) (cdr (cdr (cdr (car lst))))))
(define cddddr (lambda (lst) (cdr (cdr (cdr (cdr lst))))))

;; Others

(define list
  (lambda args
    args))

(define list?
  (lambda (lst)
    (if (null? lst)
        #t
        (and (pair? lst)
             (list? (cdr lst))))))

(define equal?
  (lambda (x y)
    (cond ((null? x) (null? y))
          ((null? y) (null? x))
          ((and (list? x) (list? y))
           (and (equal? (car x) (car y))
                (equal? (cdr x) (cdr y))))
          (else (eq? x y)))))

; Returns the length of the list
(define length
  (lambda (lst)
    (cond ((null? lst) 0)
          ((not (list? lst))
           (error "Wrong argument type provided for length"))
          (else (+ 1 (length (cdr lst)))))))

; Returns the kth element of lst (starting at 0)
(define list-ref
  (lambda (lst k)
    (cond ((null? lst)
           (error "list is shorter than index in list-ref"))
          ((not (list? lst))
           (error "Wrong argument type provided for list-ref"))
          ((zero? k) (car lst))
          (else (list-ref (cdr lst) (- k 1))))))

; returns lst with the first k items removed
(define list-tail
  (lambda (lst k)
    (cond ((null? lst)
           (error "list is shorter than index in list-tail"))
          ((not (list? lst))
           (error "Wrong argument type provided for list-tail"))
          ((zero? k) lst)
          (else (list-tail (cdr lst) (- k 1))))))

; If x is in lst, returns the sublist starting with the
; first instance of x. If x is not in lst, returns #f.
(define member
  (lambda (x lst)
    (cond ((null? lst) #f)
          ((not (list? lst))
           (error "Wrong argument type provided for member"))
          ((equal? x (car lst)) lst)
          (else (member x (cdr lst))))))

; given object and list of pairs, returns the pair whose car is the object.
; uses eq? for comparison, as specified
(define assq
  (lambda (obj lst)
    (cond ((null? lst) #f)
          ((not (list? lst))
           (error "Wrong argument type provided for assq"))
          ((not (= (length (car lst)) 2))
           (error "Invalid syntax in assq"))
          ((eq? obj (caar lst)) (car lst))
          (else (assq obj (cdr lst))))))

; Given n lists followed by one object (may or may not be a list),
; returns an (improper) list resulting from consing all the
; lists' elements to the last object.
(define append
  (lambda args
    (cond ((= (length args) 0) '())
          ((= (length args) 1) (car args))
          ((not (list? (car args)))
           (error "Wrong argument type provided for append"))
          ((null? (car args))
           (apply append (cdr args)))
          (else
           (cons (car (car args))
                 (apply append (cons (cdr (car args))
                                     (cdr args))))))))

; Reverses the given list
(define reverse
  (lambda (lst)
    (letrec ((helper
              (lambda (lst newlst)
                (if (null? lst)
                    newlst
                    (helper (cdr lst) (cons (car lst) newlst))))))
      (if (not (list? lst))
          (error "Wrong argument type provided for reverse")
          (helper lst '())))))

; apply 1-parameter function f to each element of lst
(define map
  (lambda (f lst)
    (cond ((null? lst) lst)
          ((not (list? lst))
           (error "Wrong argument type provided for map"))
          (else (cons (f (car lst)) (map f (cdr lst)))))))

; return elements x in lst for which (f x) evaluates to true (not #f)
(define filter
  (lambda (f lst)
    (cond ((null? lst) lst)
          ((not (list? lst))
           (error "Wrong argument type provided for filter"))
          ((f (car lst))
           (cons (car lst) (filter f (cdr lst))))
          (else (filter f (cdr lst))))))

; if lst = (x1 x2 ... xn),
; return (f xn (f ... (f x2 (f x1 init)) ...))
(define foldl
  (lambda (f init lst)
    (cond ((null? lst) init)
          ((not (list? lst))
           (error "Wrong argument type provided for foldl"))
          (else (foldl f (f (car lst) init) (cdr lst))))))

(define foldr
  (lambda (f init lst)
    (if (not (list? lst))
        (error "Wrong argument type provided for foldr")
        (foldl f init (reverse lst)))))














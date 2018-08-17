;; More built-in functions w.r.t. math
(load "lists.scm")

;; Helper function
(define not
  (lambda (x)
    (if x #f #t)))

;; Given at least 2 numbers, returns #t if they are all equal
(define =
  (lambda args
    (cond ((<= (length args) 1)
           (error "Wrong number of arguments provided for ="))
          ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
           (error "Wrong argument type provided for ="))
          (else
           (and (apply <= args) (apply <= (reverse args)))))))

;; Given at least 2 numbers, returns #t if each is > or = to the next
(define >=
  (lambda args
    (cond ((<= (length args) 1)
           (error "Wrong number of arguments provided for >="))
          ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
           (error "Wrong argument type provided for >="))
          (else
           (apply <= (reverse args))))))

;; Given at least 2 numbers, returns #t if each is < the next
(define <
  (lambda args
    (letrec ((diff ; makes sure no 2 subsequent items are the same
              (lambda (lst)
                (cond ((= (length lst) 2)
                       (not (= (car lst)
                               (car (cdr lst)))))
                      ((= (car lst)
                          (car (cdr lst)))
                       #f)
                      (else (diff (cdr lst)))))))
      (cond ((<= (length args) 1)
             (error "Wrong number of arguments provided for <"))
            ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
             (error "Wrong argument type provided for <"))
            (else
             (and (apply <= args) (diff args)))))))

;; Given at least 2 numbers, returns #t if each is > the next
(define >
  (lambda args
    (cond ((<= (length args) 1)
           (error "Wrong number of arguments provided for <"))
          ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
           (error "Wrong argument type provided for <"))
          (else
           (apply < (reverse args))))))

;; Given 1 number n, returns #t if it (= n 0)
(define zero?
  (lambda (n)
    (if (not (number? n))
        (error "Wrong argument type provided for zero?")
        (= n 0))))

;; Given 1 number n, returns #t if (> n 0)
(define positive?
  (lambda (n)
    (if (not (number? n))
        (error "Wrong argument type provided for positive?")
        (> n 0))))

;; Given 1 number n, returns #t if (< n 0)
(define negative?
  (lambda (n)
    (if (not (number? n))
        (error "Wrong argument type provided for negative?")
        (< n 0))))

;; Given 1 number n, returns its absolute value
(define abs
  (lambda (n)
    (cond ((not (number? n))
           (error "Wrong argument type provided for abs"))
          ((positive? n) n)
          (else (- 0 n)))))

;; Given 1 number n, returns #t if n is even
(define even?
  (lambda (n)
    (letrec ((even-pos?
              (lambda (n)
                (if (< n 2)
                    (= n 0)
                    (even-pos? (- n 2))))))
      (if (not (number? n))
          (error "Wrong argument type provided for even?")
          (even-pos? (abs n))))))

;; Given 1 number n, returns #t if n is odd
(define odd?
  (lambda (n)
    (if (not (number? n))
        (error "Wrong argument type provided for odd?")
        (even? (- n 1)))))

;; Given at least 1 number, returns the largest number
(define max
  (lambda args
    (letrec ((maxhelper
              (lambda (curmax lst)
                (cond ((null? lst) curmax)
                      ((null? curmax) ; first element
                       (maxhelper (car lst) (cdr lst)))
                      ((< curmax (car lst))
                       (maxhelper (car lst) (cdr lst)))
                      (else
                       (maxhelper curmax (cdr lst)))))))
      (cond ((= (length args) 0)
             (error "Wrong number of arguments provided for max"))
            ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
             (error "Wrong argument type provided for max"))
            (else (maxhelper '() args))))))

;; Given at least 1 number, returns the smallest number
(define min
  (lambda args
    (cond ((= (length args) 0)
           (error "Wrong number of arguments provided for min"))
          ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
           (error "Wrong argument type provided for min"))
          (else (- 0 (apply max (map (lambda (x) (- 0 x)) args)))))))

;; Returns the number r between 0 and m-1 such that
;; there exists an int k such that k*m+r = n
(define modulo
  (lambda (n m)
    (cond ((not (and (number? n) (number? m)))
           (error "Wrong argument type provided for modulo"))
          ((zero? m)
           (error "Cannot evaluate modulo 0"))
          ((positive? m)
           (cond ((and (<= 0 n) (< n m))
                  n)
                 ((negative? n)
                  (modulo (+ n m) m)) ;increase n
                 (else (modulo (- n m) m))))
          (else ; m<0
           (cond ((and (< m n) (<= n 0))
                  n)
                 ((positive? n)
                  (modulo (+ n m) m)) ;decrease n
                 (else (modulo (- n m) m)))))))

;; Round down (towards -inf)
(define floor
  (lambda (n)
    (let ((fractional-part
           (lambda (m)
             (if (>= m 0)
                 (modulo m 1)
                 (modulo m -1)))))
      (cond ((not (number? n))
             (error "Wrong argument type provided for floor"))
            ((>= n 0)
             (- n (fractional-part n)))
            ((= 0 (fractional-part n))
             n)
            (else (- n (+ 1 (fractional-part n))))))))

;; Round up (towards inf)
(define ceiling
  (lambda (n)
    (if (not (number? n))
        (error "Wrong argument type provided for ceiling")
        (- 0 (floor (- 0 n))))))

;; Returns the largest number that divides all
;; arguments, 0 if there are no arguments. Uses
;; the Euclidean algorithm.
(define gcd
  (lambda args
    (letrec ((gcd-helper ; 0 <= n <= m
              (lambda (n m)
                (if (zero? (modulo m n))
                    n
                    (gcd-two (- m n) n))))
             (gcd-two ; two args
              (lambda (n m)
                (cond ((or (not (number? n)) (not (number? m)))
                       (error "Wrong argument type provided for gcd"))
                      ((zero? n) m)
                      ((zero? m) n)
                      (else
                       (gcd-helper (min (abs n) (abs m))
                                   (max (abs n) (abs m))))))))
      (cond ((null? args) 0)
            ((not (number? (car args)))
             (error "Wrong argument type provided for gcd"))
            ((= (length args) 1)
             (car args))
            (else (foldl gcd-two 0 args))))))

;; Given n = <digit>* <dot> <digit>*, returns
;; the number resulting from removing everything after <dot>
(define truncate
  (lambda (n)
    (cond ((not (number? n))
           (error "Wrong argument type provided for truncate"))
          ((<= 0 n)
           (floor n))
          (else (ceiling n)))))

;; Returns the least common multiple of all the arguments
(define lcm
  (lambda args
    (letrec ((lcm-helper ; 0 <= n,m
              (lambda (n m original-m)
                (if (zero? (modulo m n))
                    m
                    (lcm-helper n (+ m original-m) original-m))))
             (lcm-two ; two arguments
              (lambda (n m)
                (if (or (zero? n) (zero? m))
                    0
                    (lcm-helper (abs n)
                                (abs m)
                                (abs m))))))
      (cond ((null? args) 1)
            ((not (foldl (lambda (a b) (and a b)) #t (map number? args)))
             (error "Wrong argument type provided for lcm"))
            ((= (length args) 1)
             (car args))
            (else (foldl lcm-two 1 args))))))

;; Rounds to the closest integer. If it is halfway between two integers,
;; rounds to the even one
(define round
  (lambda (n)
    (let ((frac-n (modulo n 1)))
      (cond ((not (number? n))
             (error "Wrong argument type provided for round"))
            ((< frac-n 0.5)
             (floor n))
            ((> frac-n 0.5)
             (ceiling n))
            ((even? (floor n))
             (floor n))
            (else
             (ceiling n))))))
             
;; Returns #t if x is an integer and #f otherwise.
(define integer?
  (lambda (x)
    (cond ((not (number? x)) #f)
          ((= x 0) #t)
          ((> x 0)
           (letrec ((pos-int?
                     (lambda (y)
                       (cond ((= y 0) #t)
                             ((> y 0) (pos-int? (- y 1)))
                             ((< y 0) #f)))))
             (pos-int? x)))
          ((< x 0)
           (letrec ((neg-int?
                     (lambda (y)
                       (cond ((= y 0) #t)
                             ((< y 0) (neg-int? (+ y 1)))
                             ((> y  0) #f)))))
             (neg-int? x))))))

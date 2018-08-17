(load "math.scm")
(define check-equal? equal?)

;; Gives a lazy list with elements a through b
;; Code provided in homework assignment
(define lazy-range
  (lambda (a b)
    (if (> a b)
        '()
        (cons a
              (lambda () (lazy-range (+ a 1) b))))))

;; Gives an infinite lazy list that starts at 'first' and increments by 1
(define lazy-infinite-range
  (lambda (first)
    (cons first
          (lambda () (lazy-infinite-range (+ 1 first))))))

;; Given a lazy list and an integer n return the first n elements of the list.
;; If the list contains fewer than n elements, then just return all
;; available elements
(define first-n
  (lambda (llst n)
    (cond ((= n 0) '())
          ((null? llst) '())
          (else (cons (car llst) (first-n ((cdr llst)) (- n 1)))))))

;; Given a lazy list and an integer n, returns the nth element of the list.
;; Returns false if the list has fewer than n elements.
(define nth
  (lambda (llst n)
    (cond ((null? llst) #f)
          ((= n 1) (car llst))
          (else (nth ((cdr llst)) (- n 1))))))

;; Given two lazy lists, returns a new lazy list whose ith element is the
;; sum of the ith element of llst1 and the ith element of llst2.
(define lazy-add
  (lambda (llst1 llst2)
    ; We include internal conditions to handle empty lists.
    (cons (+
           (if (null? llst1) 0 (car llst1))
           (if (null? llst2) 0 (car llst2)))
          (lambda () (lazy-add
                      (if (null? llst1) '() ((cdr llst1)))
                      (if (null? llst2) '() ((cdr llst2))))))))

;; Given a predicate and a lazy list, returns a new lazy list containing
;; elements of the original list that satisfy the predicate.
(define lazy-filter
  (lambda (predicate llst)
    (if (null? llst) '()
        (if (predicate (car llst))
            (cons (car llst) (lambda () (lazy-filter predicate ((cdr llst)))))
            ; If the predicate is not true for this item, call this function
            ; again to get the next item (and so on if none are suitable)
            (lazy-filter predicate ((cdr llst)))))))

;; Tests
;(check-equal? (nth a 4) 5)
;(check-equal? (car ((cdr (lazy-infinite-range 2)))) 3)
;(define a (lazy-infinite-range 2))
;(check-equal? (first-n a 5) '(2 3 4 5 6))
;(define b (lazy-add (lazy-infinite-range 0) (lazy-range 2 5)))
;(check-equal? (first-n b 8) '(2 4 6 8 4 5 6 7))
;(check-equal? (first-n
;               (lazy-filter (lambda (x) (= (modulo x 3) 0))
;                            (lazy-range 1 20)) 8)
;              '(3 6 9 12 15 18))
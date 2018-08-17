
(define range
  (lambda (n)
    (if (<= n 0)
        '()
        (cons n (range (- n 1))))))

"should be #t:"
(eq? 'a 'a)
(let ((x 'a))
  (eq? x 'a))
(eq? '() '())
(eq? 23 23)
(eq? 23 (+ 20 3))
(let ((x 23))
  (eq? x 23))
(eq? #t #t)
(eq? #t (eq? #t #t))
(eq? "hi" "hi")
(eq? 2 2)
(eq? 2.01 2.01)
(eq? range range)
(eq? + +)
(eq? car car)
(let ((x '(a)))
  (eq? x x))
(let ((p (lambda (x) x)))
  (eq? p p))
(eq? ((lambda () 2)) 2)
(let ((x "hi"))
  (eq? x "hi"))
(define x 3)
(define y x)
(eq? x y)
(let ((x '(1 2 3)))
  (eq? x x))
(let ((x (cons 1 2)))
  (eq? x x))

"should be #f:"
(eq? (lambda () 2) 2)
(eq? (lambda () 2) (lambda () 2))
(eq? 2 2.0)
(eq? '(1 2 3) '(1 2 3))
(eq? 'a 'b)
(eq? (cons 1 2) (cons 1 2))
(define x '(1 2 3))
(eq? x '(1 2 3))

;; problem
(define x (lambda () 1))
(eq? x (lambda () 1))
(define y (lambda () 1))
(eq? x y)

(let ((x (lambda () 1))
      (y (lambda () 1)))
  (define y 3)
  x)

(define x '())
(define y '())
(define y 3)
x







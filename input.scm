(load "math.scm")

(define bad-range
  (lambda (n)
    (if (= n 0)
        '()
        (append (bad-range (- n 1)) (list n)))))

(define good-range
  (lambda (n)
    (grhelper n '())))
(define grhelper
      (lambda (n lst)
        (if (= n 0)
            lst
            (grhelper (- n 1) (cons n lst)))))

(bad-range 10)
(good-range 10)
(good-range 100000)
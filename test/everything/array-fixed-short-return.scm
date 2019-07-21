(use-modules (gi)
             (test automake-test-lib)
             (srfi srfi-1))

(typelib-require ("Marshall" "1.0"))

(automake-test
 (let ((x (array-fixed-short-return)))
   (format #t "Output: ~S~%" x)
   (list= eqv? (short-vector->list x) '(-1 0 1 2))))

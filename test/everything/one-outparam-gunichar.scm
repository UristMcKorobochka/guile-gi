(use-modules (gi)
             (test automake-test-lib))

(typelib-require ("Everything" "1.0"))

(automake-test
 (let ((x (one-outparam-gunichar)))
   (write x) (newline)
   (char=? #\null x)))

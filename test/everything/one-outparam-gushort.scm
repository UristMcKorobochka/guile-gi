(use-modules (gi)
             (test automake-test-lib))

(typelib-require ("Everything" "1.0"))

(automake-test
 (let ((x (one-outparam-gushort)))
   (write x) (newline)
   (= 0 x)))

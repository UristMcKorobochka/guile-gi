(use-modules (gi)
             (test automake-test-lib))

(typelib-require ("Everything" "1.0"))

(automake-test
 (let ((x (const-return-gshort)))
   (write x)
   (newline)
   (eq? x 0)))

(use-modules (gi)
             (test automake-test-lib))

(typelib-require ("Everything" "1.0"))

(automake-test
 (begin
   (oneparam-gtype G_TYPE_OBJECT)
   #t))

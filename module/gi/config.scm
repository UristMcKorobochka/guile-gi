(define-module (gi config)
  #:export (gir-search-path))

(define gir-search-path
  (make-parameter
   (or (getenv "GI_GIR_PATH") "/usr/share/gir-1.0")
   (lambda (path)
     (cond
      ((string? path) (parse-path path))
      ((list? path) path)
      (else (scm-error 'wrong-type-arg "%set-gir-search-path"
                       "expected list or string" '() (list path)))))))

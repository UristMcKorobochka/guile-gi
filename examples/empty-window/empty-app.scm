(define-module (empty-window empty-app)
  #:use-module (gi)
  #:use-module (gi gio-2)
  #:use-module (gi gtk-3)
  #:use-module (empty-window empty-app-window)
  #:export(empty-app-new))

(define <EmptyApp>
  (register-type
   "EmptyApp"                           ; type name
   <GtkApplication>                     ; parent_type
   #f                                   ; No additional properties
   #f                                   ; No new signals
   #f))                                 ; No disposer func

(define (empty-app-init app)
  #f)

(define (empty-app-activate app dummy)
  (let ((win (empty-app-window-new app)))
    (send win (present))))

(define (empty-app-new)
  (let ((app
         (make-gobject
          <EmptyApp>
          ;; Alist of properties
          '(("application-id" . "org.gtk.exampleapp")
            ("flags" . 4)))))
    (connect app (activate empty-app-activate))
    app))

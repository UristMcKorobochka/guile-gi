(use-modules (gi)
             (gi compat)
             (gi util)
             (cairo)
             (srfi srfi-2))

(use-typelibs (("Gtk" "3.0") #:renamer (protect* '(init! main main-quit) 'gtk::)))

(gtk::init!)

(define* (rounded-rec #:key (xpadding 0) (ypadding 0) (radius 1)
                      (red 0.0) (blue 0.0) (green 0.0) (alpha 1.0))
    (lambda (area %ctx)
      (let ((x xpadding)
            (y ypadding)
            (width (- (get-allocated-width area) (* 2 xpadding)))
            (height (- (get-allocated-height area) (* 2 ypadding)))
            (ctx (context->cairo %ctx)))
        (and-let* (((>= width (* 2 radius)))
                   ((>= height (* 2 radius)))
                   (x+w (+ x width))
                   (y+h (+ y height))
                   (A (+ x radius))
                   (B (- x+w radius))
                   (C (+ y radius))
                   (D (- y+h radius)))
          (cairo-set-source-rgba ctx red green blue alpha)
          (cairo-new-path ctx)
          (cairo-move-to ctx A y)
          (cairo-line-to ctx B y)
          (cairo-curve-to ctx x+w y x+w y x+w C)
          (cairo-line-to ctx x+w D)
          (cairo-curve-to ctx x+w y+h x+w y+h B y+h)
          (cairo-line-to ctx A y+h)
          (cairo-curve-to ctx x y+h x y+h x D)
          (cairo-line-to ctx x C)
          (cairo-curve-to ctx x y x y A y)
          (cairo-fill ctx)
          (cairo-destroy ctx)
          #t))))

(let* ((window (make <GtkWindow> #:width-request 400 #:height-request 400))
       (drawing-area (make <GtkDrawingArea> #:parent window)))
  (connect drawing-area draw
           (rounded-rec #:xpadding 25 #:ypadding 25 #:radius 75 #:red 1.0))
  (connect window destroy (lambda _ (gtk::main-quit)))
  (show-all window))

(gtk::main)

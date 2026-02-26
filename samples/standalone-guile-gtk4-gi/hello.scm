(use-modules (gi)
             (gi repository)
             (ice-9 command-line)
             (oop goops))

(eval-when (compile load eval)
  (require "Gio" "2.0")
  (require "Gtk" "4.0")
  (load-by-name "Gio" "Application" LOAD_EVERYTHING)
  (load-by-name "Gtk" "Application" LOAD_EVERYTHING)
  (load-by-name "Gtk" "ApplicationWindow" LOAD_EVERYTHING)
  (load-by-name "Gtk" "Window" LOAD_EVERYTHING))

(define <EmptyAppWindow>
  (register-type "EmptyAppWindow" <GtkApplicationWindow> #f #f))

(define (empty-app-window-new app)
  (make <EmptyAppWindow> #:application app))

(define <EmptyApp>
  (register-type "EmptyApp" <GtkApplication> #f #f))

(define (empty-app-activate app)
  (let ((win (empty-app-window-new app)))
    (present win)))

(define (empty-app-new)
  (let ((app (make <EmptyApp>
                   #:application-id "org.gtk.exampleapp"
                   #:flags (number->application-flags 4))))
    (connect app application:activate empty-app-activate)
    app))

(define (main)
  (let ((app (empty-app-new)))
    (exit (run app (command-line)))))

(main)
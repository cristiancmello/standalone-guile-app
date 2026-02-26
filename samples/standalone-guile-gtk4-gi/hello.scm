(use-modules (gi) (gi repository))

(require "Gio" "2.0")
(require "Gtk" "4.0")
(load-by-name "Gio" "Application")
(load-by-name "Gtk" "Application")
(load-by-name "Gtk" "Widget")
(load-by-name "Gtk" "Window")
(load-by-name "Gtk" "ApplicationWindow")

(define (on-activate app)
  (let ((win (make <GtkApplicationWindow>
               #:application app
               #:title "Janela em Branco"
               #:default-width 400
               #:default-height 300)))
    (widget:show win)))

(define (main)
  (let ((app (make <GtkApplication>
               #:application-id "org.example.hello")))
    (connect app activate on-activate)
    (run app (command-line))))

(main)
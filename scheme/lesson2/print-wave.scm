(define-module (print-wave)
			   #:use-module (system foreign)
			   #:use-module	(ice-9 format)
			   #:export (sf-open-write
						  sf-write-float
						  sf-close))

(define *SFM-WRITE* #x20)

(define snd-info-types (list size_t int int int int int))

(define sndfile (dynamic-link "/usr/lib/x86_64-linux-gnu/libsndfile.so"))
(define sf-open (pointer->procedure '*
  (dynamic-func "sf_open" sndfile)
  (list '* int '*)))
(define sf-write-int_ (pointer->procedure size_t
  (dynamic-func "sf_write_int" sndfile)
  (list '* '* size_t)))

(define sf-write-float_ (pointer->procedure size_t
  (dynamic-func "sf_write_float" sndfile)
  (list '* '* size_t)))


(define sf-write-double_ (pointer->procedure size_t
  (dynamic-func "sf_write_double" sndfile)
  (list '* '* size_t)))

(define sf-write-sync_ (pointer->procedure int
  (dynamic-func "sf_write_sync" sndfile)
  (list '*)))

(define sf-close (pointer->procedure void
  (dynamic-func "sf_close" sndfile)
  (list '* )))

(define (sf-open-write filename rate)
  (sf-open (string->pointer filename) *SFM-WRITE*
		   (make-c-struct snd-info-types
		   (list 0 rate 1 #x10006 0 0))))

(define (sf-write-float snd-file frame size)
  (sf-write-float_ snd-file (bytevector->pointer frame) size))

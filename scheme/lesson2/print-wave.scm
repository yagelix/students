(define-module (print-wave)
			   #:use-module (system foreign)
			   #:use-module	(ice-9 format)
			   #:export ( *SFM-WRITE*
						  snd-info-types
						  sf-open
						  sf-write-float
						  sf-write-double
						  sf-write-sync
						  sf-close))

(define *SFM-WRITE* #x20)

(define snd-info-types (list size_t int int int int int))

(define sndfile (dynamic-link "libsndfile"))
(define sf-open (pointer->procedure '*
  (dynamic-func "sf_open" sndfile)
  (list '* int '*)))

(define sf-write-float (pointer->procedure size_t
  (dynamic-func "sf_write_float" sndfile)
  (list '* '* size_t)))


(define sf-write-double (pointer->procedure size_t
  (dynamic-func "sf_write_double" sndfile)
  (list '* '* size_t)))

(define sf-write-sync (pointer->procedure int
  (dynamic-func "sf_write_sync" sndfile)
  (list '*)))

(define sf-close (pointer->procedure void
  (dynamic-func "sf_close" sndfile)
  (list '* )))


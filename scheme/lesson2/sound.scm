(define-module (sound)
			   #:use-module (rnrs bytevectors)
			   #:use-module (print-wave)
			   #:use-module (ice-9 format)
			   #:export (make-snd-writer *SAMPLERATE*))

(define *SAMPLERATE* 44100)

(define (make-sample t)
  (make-bytevector (* 4 *SAMPLERATE* t) 0))

(define (sample-set! sample at value)
  (bytevector-ieee-single-native-set! sample (* 4 at) value))
 

(define (make-snd-writer snd-file)
  (display "Making snd-writer...\n")
  (let ((frame (make-sample 1))
		(cursor 0))
	(lambda (value)
	  (cond
		((eq? value 'off)
		 (cond 
		   ((> cursor 0)
			(format #t "Dumping to file ~d..\n" cursor)
			(sf-write-float snd-file frame cursor))))
		(else
		  (when (>= cursor *SAMPLERATE*)
			(display (format #f "dumping ~d...\n" cursor))
			(sf-write-float snd-file frame cursor)
			(set! cursor 0))
		  (sample-set! frame cursor value)
		  (set! cursor (1+ cursor)))))))
 

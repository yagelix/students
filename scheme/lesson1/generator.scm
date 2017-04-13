(use-modules (ice-9 format)
			 (rnrs bytevectors)
			 ;; for C types
			 (system foreign))

(define *SAMPLERATE* 44100)
(define *PI* 3.141593)
(define dt (/ 1.0 *SAMPLERATE*))

(define (oscillator freq len)
  (let
	((frame
	   (make-bytevector (* 4 *SAMPLERATE* len) 0)))
	(do ((t 0 (+ dt t))
		 (i 0 (+ 1 i)))
	  ((>= t len ))
	  (bytevector-ieee-single-native-set! frame (* i 4) (sin (* 2 *PI* freq t))))
	frame))

;; constant for sf_open write semantics
;;
(define *SFM-WRITE* #x20)

;; structure for sf_open
(define snd-info-types (list size_t int int int int int))



(define sndfile (dynamic-link "libsndfile"))
;; sndfile* x = sf_open("my_file.wav", SF_READ, WAV_32)
;; (define x (sf-open "xxx" 123 123))

(define sf-open (pointer->procedure '*
									(dynamic-func "sf_open" sndfile)
									(list '* int '*)))
(define sf-close (pointer->procedure void
									 (dynamic-func "sf_close" sndfile)
									 (list '*)))
(define sf-write-float (pointer->procedure int
										   (dynamic-func "sf_write_float" sndfile)
										   (list '* '* size_t)))

;; writing simple file
(define snd-file (sf-open (string->pointer "result.wav") *SFM-WRITE*
						  (make-c-struct snd-info-types
										 ;; form structure for file format
										 ;; as 44,1 KHz 16-bit float 1-channel WAV
										 (list 0 *SAMPLERATE* 1 #x10006 0 0))))
;; write note A of the 1st octave (440Hz) length 3 sec
(sf-write-float snd-file (bytevector->pointer (oscillator 440 3)) (* *SAMPLERATE* 3))
;; close file
(sf-close snd-file)


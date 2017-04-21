(add-to-load-path (dirname (current-filename)))
(use-modules (print-wave)
			 (sound)
			 (rnrs bytevectors))

(define snd-file (sf-open-write "test.wav" 44100))

(define writer (make-snd-writer snd-file))
;; qweqwedd
;;q qwe qwe
;; qweqwe
;;
(define (square x) (* x x))
;; release quotent e^(- gamma *t)
(define gamma (/ 3 *SAMPLERATE*))
;; frequency e^(i * omega * t)
(define omega (/ (* 2 3.1415 440) *SAMPLERATE*))
;; linear quotents
;;
;; f_1 = a * f_1 + b * f_2
;; f_2 = a * f_2 - b * f_1
;;
;;
(define a (* (exp (- gamma)) (cos omega)))
(define b (* (exp (- gamma)) (sin omega)))

(format #t "a=~f b=~f\n" a b)


(define (write-sine snd-writer t f1 f2)
  (cond 
	((> t 0)
	 (snd-writer f1)
	 (write-sine snd-writer (- t (/ 1 *SAMPLERATE*))
				 (+ (* a f1) (* b f2))
				 (- (* a f2) (* b f1))))
	(else (snd-writer 'off))))

(write-sine writer 2.5 1 0)
(sf-close snd-file)


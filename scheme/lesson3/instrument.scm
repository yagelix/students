(define-module (instrument)
			   #:use-module (sound)
			   #:use-module (srfi srfi-9)
			   #:export (make-simple-instrument
						   make-sine-oscillator
						   make-saw
						   simple-envelope
						   *default-instrument*
						   ))

(define *PI* 3.141592)
(define *2PI* 6.283185)

;; make-instrument  (oscillator envelope) -> freq amp len -> t stop -> value
;; 
(define (make-simple-instrument) 
  (lambda (oscillator envelope)
	(lambda (freq amp len)
	  (let  ((osc (oscillator freq))
			 (env (envelope amp len)))
		(lambda (t stop) 
		   (* (osc t) (env t stop)))))))


;; make-oscillator : freq -> t -> value
(define (make-sine-oscillator)
  (lambda (freq)
	  (lambda (t)
		(sin (* *2PI* freq t)))))

(define (make-saw)
  (lambda (freq)
	(lambda (t)
	  (* freq (floor-remainder t (/ 1.0 freq))))))

(define (make-dist-oscillator shift dist)
  (lambda (freq)
	(lambda (t)
	  (tanh (+ shift (* dist (sin (* *2PI* freq t))))))))


(define (linear-amp t1 dt amp1 amp2 t)
  (+ amp1 (* (- amp2 amp1) (/ (- t  t1) dt))))


;; simple envelope generator
;; simple-envelope attack decay sustain release -> amp len -> t stop -> value
(define (simple-envelope attack decay sustain release)
  (lambda (amp len)
	(let ((release-at 0)
		  (state 'on))
	  (lambda (t stop)
		(let ((value (cond
					   ;; after all programm call 'stop
					   ((> t (+ len release))
						(stop) 0)
					   ((> t len) (linear-amp len release release-at 0.0 t))
					   ((< t attack) (* amp (/ t attack)))
					   ((< t (+ attack decay)) (* amp (linear-amp attack decay 1.0 sustain t )))
					   (else (* amp sustain))) ))
		  (when (and (eq? state 'on) (> t len))
			(set! state 'off)
			(set! release-at value))
;;		  (format #t "Envelope: t=~,7f ~,5f\n" t value)
		  value)))))


(define *default-instrument* ((make-simple-instrument)
							  (make-dist-oscillator 1.0 10)
							  (simple-envelope 0.01 0.2 0. 1.0)))






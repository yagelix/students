(define-module (changen)
			   #:use-module (instrument)
			   #:use-module (srfi srfi-1)
			   #:export (make-channel make-generator))

(define (apply-filters filter-chain input)
  (fold (lambda (flt result)
		  (flt result) ) input filter-chain)) 

;; channel
;; channel accepts the following commands
;; 'select-instrument instrument  -- selects current instrument for new generators
;; 'instrument -- returns current instrument
;; 'insert-filter  -- inserts filter
;; 'amp value -- set current channel amplification
;; 'mix value -- add value to the current time frame
;; 'next -- process output with filters and return it



(define (make-channel)
  (let ((instrument *default-instrument*)
		(filter-chain '())
		(current-amp 0.3)
		(current-output 0)
		(current-input 0))
	(display "Creating channel...\n")
	(lambda (cmd . args)
	  (case cmd
		;; set current instrument
		((select-instrument)
		 (set! instrument (car args)))
		((instrument) instrument)
		((insert-filter)
		 (set! filter-chain (cons (car args) filter-chain)))
		((amp)
		 (set! current-amp (car args)))
		((mix)
		 (set! current-input (+ current-input (car args))))
		((next) 
		 ;; return current (amplified) output 
		 (set! current-output (apply-filters filter-chain current-input))
		 (set! current-input 0)
		 (* current-amp current-output))
		(else
		  (format #t "unknown command for channel ~a" cmd))))))
  


(define (make-generator channel freq amp len) 
  (let ((state 'on)
		(instrument ((channel 'instrument) freq amp len))
		(ts 0))
	(let ((stopper (lambda () (set! state 'off))))
	  (lambda (cmd . args)
		(case cmd 
		  ;; get next frame
		  ((next) 
		   	(channel 'mix
						   (if (eq? state 'on)
							 ;; continue instrument output generation
							 ;; and send it to
							 (instrument ts stopper) 0))
			(set! ts (+ ts (car args))))

		  ((state) state)
		  ((channel) channel)
		  ((ctrl)
		   ;; nothing implemented yet, so
		   ;; returning zero
		   0))))))
  

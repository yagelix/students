(define-module (musasm)
			   #:use-module (srfi srfi-1)
			   #:use-module (sound)
			   #:use-module (instrument)
			   #:use-module (changen)
			   #:export (musasm-processor))


(define dt (/ 1.0 *SAMPLERATE*))

;; musasm is very simple
;; the asm is a sequence of events
;; in form (time event params)
;; simple events are:
;; 


;; process generators and write
(define (step-generators dt generators)
  (for-each (lambda (gen)
			  (gen 'next dt)) generators))


;; process channels output and write it to snd-writer
(define (step-channels channels snd-writer)
	(snd-writer (fold (lambda (chan result)
						 (+ result (chan 'next))) 0 channels)))


(define (process-timed-events t channels generators writer evtlist)
  (display (format #f "processing-timed-events at t=~f generators: ~d\n" t (length generators)))
  (newline)
  (cond
	((null? evtlist)
	 ;; if there is no more events --just continue steps on generators
	 (format #t "no more events -- processing generators at ~f\n" t)
	 (while (> (length generators) 0)
			(step-generators dt generators)
			(step-channels channels writer)
			(set! t (+ t dt))
			(set! generators (cleanup-gen generators)))
	 (format #t "no more generators at ~f \n" t))
	(else 
	  (let ((next-t (caar evtlist))
			(events (cdar evtlist)))
		;; main cycle
		(display (format #f " next t is ~f\n" next-t))
		(display "Generating frame\n")
		(while (< t next-t)
			   ;; generate  and notify channels
			   (step-generators dt generators)
			   ;; write channels result
			   (step-channels channels writer)
			   (set! t (+ t dt)))
		(set! generators (process-event-group (cleanup-gen generators) events))
		(process-timed-events t channels generators writer (cdr evtlist))))))

(define (process-event-group genlist evtlist)
  (cond
	((null? evtlist) genlist)
	(else 
	  (let* ((event (car evtlist))
			 (channel (car event))
			 (cmd (cadr event))
			 (args (cddr event)))
		(case cmd
		  ((note) 
		   ;;make note generator
		   (let* ((freq (car args))
				  (amp (cadr args))
				  (len (caddr args))
				  (gen (make-generator channel freq amp len)))
			 ;; and start note
			 (set! genlist (cons gen genlist))))
		  ((instrument)
		   ;; change instrument
		   (display "Changing instrument...\n")
		   (channel 'instrument args))
		  ((amp)
		   ;; channel amplification
		   (channel 'amp args))
		  ((ctrl)
		   (for-each (lambda (gen)
					   (when 
						 (eq? (gen 'channel) channel)
						 (gen 'ctrl args))) genlist))
		  (else 
			(format #t "unknown command ~a\n" cmd)))
		(process-event-group genlist (cdr evtlist))))))

;; cleanup unused generators
(define (cleanup-gen genlist) 
  (filter (lambda (gen)
			(eq? (gen 'state) 'on)) genlist))

;; event stream -> group by t and sort by t events
(define (sort-events eventlist)
  (letrec ((group-by-t (lambda (lst last current-group)
						 (cond
						   ;; list is empty
						   ((null? lst) 
							;; release group if not empty
							(if (null? current-group) '()
							  ;; return simple pair t . event-list
							  (list (cons last current-group))))
						   (else
							 (let* ((element (car lst))
									(t (car element))
									(evt (cdr element)))
							   (cond
							   ((= t last)
								;; same time stamp -- > group
								(group-by-t (cdr lst) last (cons evt current-group) ))
							   (else 
								 ;; release group if not empty
								 (if (null? current-group)
								   (group-by-t (cdr lst) t (list evt))
								   (cons (cons last current-group) (group-by-t (cdr lst) t (list evt))))))))))))
	
	(group-by-t
	  (sort-list! eventlist
				  (lambda (a b) (< (car a) (car b)))) -1 '())))

(define (musasm-processor channels snd-file events)
  (let ((snd-writer (make-snd-writer snd-file))
		(sorted-events (sort-events events)))
	(display "Sorted events: ")
	(display sorted-events)
	(newline) 
	(process-timed-events 0 channels '() snd-writer sorted-events)

	;; finalize writer
  (snd-writer 'off)))

 
 

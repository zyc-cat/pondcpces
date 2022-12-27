(define (domain cube)
 (:predicates 
 (x-pos ?pos)
    
 
  )(:action up-x
                   :parameters ()
                   :precondition ()
                   :effect (and
                 (when (and (x-pos p0)) (and (x-pos p1) (not (x-pos p0)) (not (x-pos p2))))
                 (when (and (x-pos p1)) (and (x-pos p2) (not (x-pos p0)) (not (x-pos p1))))
                 (when (and (x-pos p2)) (and  (not (x-pos p0)) (not (x-pos p1)) (x-pos p2)))
))
(:action down-x
                   :parameters ()
                   :precondition ()
                   :effect (and (when (x-pos p0) (and (x-pos p0) (not (x-pos p1)) (not (x-pos p2))))
                    (when (and (x-pos p1)) (and (x-pos p0) (not (x-pos p1)) (not (x-pos p2))))
                    (when (and (x-pos p2)) (and (x-pos p1) (not (x-pos p0)) (not (x-pos p2))))
                    
                    
))
)

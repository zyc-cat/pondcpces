(define (domain cube)
 (:predicates 
 (x-pos ?pos)
    (y-pos ?pos) 
               (z-pos ?pos)
 
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
(:action up-y
                   :parameters ()
                   :precondition ()
                   :effect (and 
                 (when (and (y-pos p0)) (and (y-pos p1) (not (y-pos p0)) (not (y-pos p2))))
                 (when (and (y-pos p1)) (and (y-pos p2) (not (y-pos p0)) (not (y-pos p1))))
                 (when (and (y-pos p2)) (and (not (y-pos p0)) (not (y-pos p1)) (y-pos p2)))
                 
))
(:action down-y
                   :parameters ()
                  :precondition ()
                   :effect (and (when (y-pos p0) (and (y-pos p0) (not (y-pos p1)) (not (y-pos p2))))
                    (when (and (y-pos p1)) (and (y-pos p0) (not (y-pos p1)) (not (y-pos p2))))
                    (when (and (y-pos p2)) (and (y-pos p1) (not (y-pos p0)) (not (y-pos p2))))
))
(:action up-z
                   :parameters ()
                   :precondition ()
                   :effect (and 
                 (when (and (z-pos p0)) (and (z-pos p1) (not (z-pos p0)) (not (z-pos p2))))
                 (when (and (z-pos p1)) (and (z-pos p2) (not (z-pos p0)) (not (z-pos p1))))
                 (when (and (z-pos p2)) (and (not (z-pos p0)) (not (z-pos p1)) (z-pos p2)))
))
(:action down-z
                   :parameters ()
                   :precondition ()
                   :effect (and (when (z-pos p0) (and (z-pos p0) (not (z-pos p1)) (not (z-pos p2))))
                    (when (and (z-pos p1)) (and (z-pos p0) (not (z-pos p1)) (not (z-pos p2))))
                    (when (and (z-pos p2)) (and (z-pos p1) (not (z-pos p0)) (not (z-pos p2))))
                    
                    
))
)

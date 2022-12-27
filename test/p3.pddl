(define (problem cube5)
(:domain cube)
(:requirements :strips :equality :typing :conditional-effects :disjunctive-preconditions)
(:objects p0 p1 p2)
(:init 
 (and
;(oneof (x-pos p0)
;(x-pos p1)
;(x-pos p2)
;)

(or
(x-pos p0)
(x-pos p1)
(x-pos p2)
)

)
 )
   (:goal (and (x-pos p1) )))

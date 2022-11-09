(define (domain btc)
  (:requirements :strips :equality :typing :conditional-effects :disjunctive-preconditions)
  (:types package bomb )
  (:predicates
    (in ?p - package ?b - bomb)
    (defused ?b - bomb)
   )

  (:action senseP 
   :parameters (?p - package ?b - bomb)
   :precondition ()
   :observe ((in ?p ?b))
   )
   

  (:action dunk	
   :parameters (?p - package ?b - bomb)
   :precondition ()           
   :effect (when (in ?p ?b)
      (defused ?b)))

)
  

(define (domain Rover)
(:requirements :typing)
(:types rover waypoint store camera mode lander objective)

(:predicates (at ?x - rover ?y - waypoint);rover在y
             (at_lander ?x - lander ?y - waypoint); 登陆lander在y
             (can_traverse ?r - rover ?x - waypoint ?y - waypoint); rover从x到y
             (equipped_for_imaging ?r - rover); rover是否配有图像
	     (equipped_for_soil_analysis ?r - rover); rover是否与solid分析
             (equipped_for_rock_analysis ?r - rover); rover是否与rock分析
             (empty ?s - store); s为空
             ; r是否有进行了分析
             (have_rock_analysis ?r - rover); ?w - waypoint)
             (have_soil_analysis ?r - rover); ?w - waypoint)
             ; store是否存满
             (full ?s - store)
             ; rover的相继完成了目标对焦
	     (calibrated ?c - camera ?r - rover ?w - waypoint ?o - objective) 
	     (supports ?c - camera ?m - mode)
             (available ?r - rover)
             (visible ?w - waypoint ?p - waypoint)
             (have_image ?r - rover ?o - objective ?m - mode)
             (communicated_image_data ?o - objective ?m - mode)
             (communicated_soil_data); ?w - waypoint)
             (communicated_rock_data); ?w - waypoint)
 	     (at_soil_sample ?w - waypoint)
	     (at_rock_sample ?w - waypoint)
   	     (visible_from ?o - objective ?w - waypoint)
	     (store_of ?s - store ?r - rover)
	     (calibration_target ?i - camera ?o - objective)
	     (on_board ?i - camera ?r - rover)
	     (channel_free ?l - lander)

)

	
(:action navigate
:parameters (?x - rover ?y - waypoint ?z - waypoint ?c - camera ?p - objective) 
:precondition (and (can_traverse ?x ?y ?z) (available ?x) (at ?x ?y) 
                (visible ?y ?z)
	    )
:effect (and (not (at ?x ?y)) (at ?x ?z)
;	(when (calibrated ?c ?x ?z ?p) 
(not (calibrated ?c ?x ?z ?p))
;)
		)
	
)


(:action calibrate
 :parameters (?r - rover ?i - camera ?t - objective ?w - waypoint)
 :precondition 
(and 
	(equipped_for_imaging ?r) ;not need
	(calibration_target ?i ?t) ; not need
	(at ?r ?w) 
;	(visible_from ?t ?w) ; this one may should not be comment
	(on_board ?i ?r)) ;not need
 :effect ;(when (visible_from ?t ?w)  ; this is not always true in initial state, and won't be changed by effect
(calibrated ?i ?r ?w ?t);)
)


(:action take_image
 :parameters (?r - rover ?p - waypoint ?o - objective ?i - camera ?m - mode)
 :precondition (and (calibrated ?i ?r ?p ?o)
			 (on_board ?i ?r)
                      (equipped_for_imaging ?r)
                      (supports ?i ?m)
;			 (visible_from ?o ?p)
                     (at ?r ?p)
               )
 :effect (and (when (visible_from ?o ?p) (have_image ?r ?o ?m))
              (not (calibrated ?i ?r ?p ?o)) 
         )
;)
		
)



(:action communicate_image_data
 :parameters (?r - rover ?l - lander ?o - objective ?m - mode ?x - waypoint ?y - waypoint)
 :precondition (and (at ?r ?x)
                    (at_lander ?l ?y)
	            (have_image ?r ?o ?m)
                    (visible ?x ?y)
;                    (available ?r)
;                    (channel_free ?l)
               )
 :effect (and 
;(not (available ?r))
;(not (channel_free ?l))(channel_free ?l)
(communicated_image_data ?o ?m)
;(available ?r)
          )
)

(:action sample_soil
:parameters (?x - rover ?s - store ?p - waypoint)
:precondition (and (at ?x ?p) 
                  ; (at_soil_sample ?p) 
                   (equipped_for_soil_analysis ?x) 
                   (store_of ?s ?x) 
                   (empty ?s)
		   (not (full ?s))
		)
:effect (and (when (at_soil_sample ?p) 
                   (and (not (empty ?s)) 
                        (full ?s) 
                        (have_soil_analysis ?x); ?p) 
                        (not (at_soil_sample ?p))
                   )
              ) 
         )
)

(:action sample_rock
:parameters (?x - rover ?s - store ?p - waypoint)
:precondition (and (at ?x ?p) 
             ;      (at_rock_sample ?p) 
                   (equipped_for_rock_analysis ?x) 
                   (store_of ?s ?x)
                   (empty ?s)
		)
:effect (and (when (at_rock_sample ?p)
                   (and	(not (empty ?s)) 
                        (full ?s) 
                        (have_rock_analysis ?x); ?p) 
                        (not (at_rock_sample ?p))
                   )
              )
         )
)

(:action drop
:parameters (?x - rover ?y - store)
:precondition (and 
		(store_of ?y ?x) 
             ;   (full ?y)
	      ;  (not (empty ?y))
	       )
:effect ;(when (and (full ?y) (not (empty ?y)))
              (and (not (full ?y)) 
                   (empty ?y)
	      )
        ; )
)

(:action communicate_soil_data
 :parameters (?r - rover ?l - lander ;?p - waypoint 
              ?x - waypoint ?y - waypoint)
 :precondition (and (at ?r ?x)
                    (at_lander ?l ?y)
                    (have_soil_analysis ?r); ?p) 
                    (visible ?x ?y)
                    (available ?r)
                    (channel_free ?l)
                )
 :effect ;(and (not (available ?r))
         ;     (not (channel_free ?l))
         ;     (channel_free ?l)
	      (communicated_soil_data)
         ;     (available ?r)
	 ; )
)

(:action communicate_rock_data
 :parameters (?r - rover ?l - lander ;?p - waypoint 
		?x - waypoint ?y - waypoint)
 :precondition (and (at ?r ?x)
                    (at_lander ?l ?y)
                    (have_rock_analysis ?r); ?p)
                    (visible ?x ?y)
                    (available ?r)
                    (channel_free ?l)
                )
 :effect ;(and (not (available ?r))
         ;     (not (channel_free ?l))
         ;     (channel_free ?l)
              (communicated_rock_data)
         ;     (available ?r)
         ;)
)
)

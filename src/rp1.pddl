(define (problem roverprob1234) (:domain Rover)
(:objects
        general - lander
        colour high_res low_res - mode
        rover0 - rover
        waypoint0 waypoint1 waypoint2 waypoint3 - waypoint
        camera0 - camera
        objective1 - objective
        )
(:init
(and
	 (visible waypoint1 waypoint0) 
	 (visible waypoint2 waypoint0)
	 (visible waypoint3 waypoint0)
        (visible waypoint0 waypoint1)

        (visible waypoint0 waypoint2)
        (visible waypoint2 waypoint1)
        (visible waypoint1 waypoint2)

        (visible waypoint0 waypoint3)
        (visible waypoint3 waypoint1)
        (visible waypoint1 waypoint3)
        (visible waypoint3 waypoint2)
        (visible waypoint2 waypoint3)
        (at_lander general waypoint0)
	(not (at_lander general waypoint1))
 	(not (at_lander general waypoint2))
 	(not (at_lander general waypoint3))
    
	   (channel_free general)
        (at rover0 waypoint3)
	(not (at rover0 waypoint0))
	(not (at rover0 waypoint1))
	(not (at rover0 waypoint2))
        (available rover0)
        (equipped_for_imaging rover0)
        (can_traverse rover0 waypoint3 waypoint0)
        (can_traverse rover0 waypoint0 waypoint3)
        (can_traverse rover0 waypoint3 waypoint1)
        (can_traverse rover0 waypoint1 waypoint3)
        (can_traverse rover0 waypoint1 waypoint2)
        (can_traverse rover0 waypoint2 waypoint1)
	(not (can_traverse rover0 waypoint1 waypoint0))
	(not (can_traverse rover0 waypoint0 waypoint1))
	(not (can_traverse rover0 waypoint2 waypoint0))
	(not (can_traverse rover0 waypoint0 waypoint2))
	(not (can_traverse rover0 waypoint2 waypoint3))
	(not (can_traverse rover0 waypoint3 waypoint2))

	(not (have_image rover0 objective1 colour))
	(not (have_image rover0 objective1 high_res))
	(not (have_image rover0 objective1 low_res))

        (on_board camera0 rover0)
        (calibration_target camera0 objective1)
        (supports camera0 colour)
        (supports camera0 high_res)
        (not (supports camera0 low_res))
     	(not (calibrated camera0 rover0 waypoint0 objective1))
 	 (not (calibrated camera0 rover0 waypoint1 objective1))
	(not (calibrated camera0 rover0 waypoint2 objective1))
 	 (not (calibrated camera0 rover0 waypoint3 objective1))
	(not (communicated_image_data objective1 high_res))
	(not (communicated_image_data objective1 low_res))
	(not (communicated_image_data objective1 colour))
	(not (have_soil_analysis rover0))
	(not (have_rock_analysis rover0))
	(not (communicated_soil_data))
	(not (communicated_rock_data))
	(not (at_soil_sample waypoint1))
	(not (at_soil_sample waypoint0))
	(not (at_soil_sample waypoint2))
	(not (at_soil_sample waypoint3))
	(not (at_rock_sample waypoint0))
	(not (at_rock_sample waypoint1))
	(not (at_rock_sample waypoint3))
	(not (at_rock_sample waypoint2))
         (not (visible_from objective1 waypoint0))
         (not (visible_from objective1 waypoint1))
         (not (visible_from objective1 waypoint3))


         (visible_from objective1 waypoint2)

))

(:goal
(communicated_image_data objective1 high_res)



)
)


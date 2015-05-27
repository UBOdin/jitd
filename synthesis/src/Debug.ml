
let active_modes: string list ref = ref [];;

let activate_mode (mode:string): unit = 
    if (not ( List.mem mode (!active_modes) )) then 
    	active_modes := mode :: !active_modes
    else ()
;;

let deactivate_mode (mode:string): unit =
	active_modes := 
		List.fold_right (fun x ret -> 
			if x = mode then ret else x :: ret
		) !active_modes []
;;

let active (mode:string):bool = List.mem mode !active_modes
;;

let print (mode:string) (msg:string):unit = if active mode then print_endline msg else ()
;;

let do_if (mode:string) (op: (unit -> unit)): unit = if active mode then op () else ();;
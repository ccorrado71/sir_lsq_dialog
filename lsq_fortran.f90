module lsq_module
    use iso_c_binding
    implicit none
    
contains
    
    ! Subroutine to get initial LSQ parameters
    subroutine lsq_get_parameters(refinement_type, damping_factor, reflections_cutoff, &
                                   num_cycles, weighting_scheme, weight_params, &
                                   refine_weight, num_observations, percent_observations, &
                                   num_parameters, ratio, num_weight_params, ier) bind(C, name="lsq_get_parameters")
        integer(c_int), intent(out) :: refinement_type
        real(c_double), intent(out) :: damping_factor
        integer(c_int), intent(out) :: reflections_cutoff
        integer(c_int), intent(out) :: num_cycles
        integer(c_int), intent(out) :: weighting_scheme
        real(c_double), intent(out) :: weight_params(10)
        integer(c_int), intent(out) :: refine_weight
        integer(c_int), intent(out) :: num_observations
        real(c_double), intent(out) :: percent_observations
        integer(c_int), intent(out) :: num_parameters
        real(c_double), intent(out) :: ratio
        integer(c_int), intent(out) :: num_weight_params(18)
        integer(c_int), intent(out) :: ier
        
        ! Set default/example parameters
        refinement_type = 0  ! 0=Diagonal, 1=FullMatrix, 2=SFCOnly
        damping_factor = 0.7d0
        reflections_cutoff = 3
        num_cycles = 10
        weighting_scheme = 2  ! Index of weighting scheme (0-based)
        
        ! Initialize weight parameters with example values
        weight_params(1) = 1.0d0
        weight_params(2) = 0.5d0
        weight_params(3) = 0.25d0
        weight_params(4) = 0.0d0
        weight_params(5) = 0.0d0
        weight_params(6) = 0.0d0
        weight_params(7) = 0.0d0
        weight_params(8) = 0.0d0
        weight_params(9) = 0.0d0
        weight_params(10) = 0.0d0
        
        refine_weight = 1  ! 0=false, 1=true
        
        ! Observations & Parameters
        num_observations = 1234
        percent_observations = 92.5d0
        num_parameters = 45
        ratio = 27.42d0
        
        ! Number of weight parameters per scheme (18 schemes)
        num_weight_params(1) = 1    ! Scheme 1: needs P(1)
        num_weight_params(2) = 3    ! Scheme 2: needs P(1), P(2), P(3)
        num_weight_params(3) = 1    ! Scheme 3: needs P(1)
        num_weight_params(4) = 1    ! Scheme 4: needs P(1)
        num_weight_params(5) = 0    ! Scheme 5: no parameters (uses Sigma(Fo))
        num_weight_params(6) = 0    ! Scheme 6: no parameters (W=1.0)
        num_weight_params(7) = 2    ! Scheme 7: needs P(1), P(2)
        num_weight_params(8) = 1    ! Scheme 8: needs P(1)
        num_weight_params(9) = 0    ! Scheme 9: no parameters
        num_weight_params(10) = 0   ! Scheme 10: no parameters
        num_weight_params(11) = 4   ! Scheme 11: needs P(1), P(2), P(3), P(4)
        num_weight_params(12) = 2   ! Scheme 12: needs P(1), P(2)
        num_weight_params(13) = 4   ! Scheme 13: needs P(1), P(2), P(3), P(4)
        num_weight_params(14) = 3   ! Scheme 14: needs P(1), P(2), P(3)
        num_weight_params(15) = 3   ! Scheme 15: needs P(1), P(2), P(3)
        num_weight_params(16) = 0   ! Scheme 16: complex, set to 0 for now
        num_weight_params(17) = 3   ! Scheme 17: needs P(1), P(2), P(3)
        num_weight_params(18) = 5   ! Scheme 18: needs P(1), P(2), P(3), P(4), P(5)
        
        ! Set error code (0 = success, non-zero = error)
        ier = 0
        
        print *, "Fortran: Returning initial LSQ parameters"
        
    end subroutine lsq_get_parameters
    
    ! Subroutine to execute LSQ calculation with given parameters
    subroutine lsq_execute(refinement_type, damping_factor, reflections_cutoff, &
                           num_cycles, weighting_scheme, weight_params, &
                           refine_weight) bind(C, name="lsq_execute")
        integer(c_int), intent(in), value :: refinement_type
        real(c_double), intent(in), value :: damping_factor
        integer(c_int), intent(in), value :: reflections_cutoff
        integer(c_int), intent(in), value :: num_cycles
        integer(c_int), intent(in), value :: weighting_scheme
        real(c_double), intent(in) :: weight_params(10)
        integer(c_int), intent(in), value :: refine_weight
        
        character(len=20) :: ref_type_str
        
        ! Decode refinement type
        select case(refinement_type)
            case(0)
                ref_type_str = "Diagonal"
            case(1)
                ref_type_str = "Full Matrix"
            case(2)
                ref_type_str = "S.F.C. only"
            case default
                ref_type_str = "Unknown"
        end select
        
        print *, "========================================"
        print *, "Fortran: Executing LSQ Refinement"
        print *, "========================================"
        print *, "Refinement Type: ", trim(ref_type_str)
        print *, "Damping Factor: ", damping_factor
        print *, "Reflections Cutoff: ", reflections_cutoff
        print *, "Number of Cycles: ", num_cycles
        print *, "Weighting Scheme Index: ", weighting_scheme
        print *, "Weight Parameters:"
        print *, "  P(1) = ", weight_params(1)
        print *, "  P(2) = ", weight_params(2)
        print *, "  P(3) = ", weight_params(3)
        print *, "  P(4) = ", weight_params(4)
        print *, "  P(5) = ", weight_params(5)
        print *, "Refine Weight Parameters: ", refine_weight == 1
        print *, "========================================"
        print *, "Calculation completed successfully!"
        print *, "========================================"
        
    end subroutine lsq_execute
    
end module lsq_module

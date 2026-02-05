module lsq_module
    use iso_c_binding
    implicit none
    
contains
    
    ! Subroutine to get initial LSQ parameters
    subroutine lsq_get_parameters(refinement_type, damping_factor, reflections_cutoff, &
                                   num_cycles, weighting_scheme, weight_params, &
                                   refine_weight, num_observations, percent_observations, &
                                   num_parameters, ratio, num_weight_params, &
                                   all_weight_params, num_atoms, ier) bind(C, name="lsq_get_parameters")
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
        real(c_double), intent(out) :: all_weight_params(10, 18)
        integer(c_int), intent(out) :: num_atoms
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
        
        ! Initialize all weight parameters matrix (10 params x 18 schemes)
        ! Each column represents the parameters for one weighting scheme
        all_weight_params = 0.0d0  ! Initialize all to zero
        
        ! Scheme 1: W=Fo/P(1) if Fo >= P(1); W=1/W otherwise
        all_weight_params(1, 1) = 1.0d0
        
        ! Scheme 2: W=1/(P(1)+P(2)*Fo+P(3)*Fo^2)
        all_weight_params(1, 2) = 1.0d0
        all_weight_params(2, 2) = 0.5d0
        all_weight_params(3, 2) = 0.25d0
        
        ! Scheme 3: W=(Sin(Theta)/Lambda)^P(1)
        all_weight_params(1, 3) = 2.0d0
        
        ! Scheme 4: W=(Lambda/Sin(Theta))^P(1)
        all_weight_params(1, 4) = 1.0d0
        
        ! Scheme 5 & 6: no parameters (already initialized to 0)
        
        ! Scheme 7: W=1/(1.+((Fo-P(2))/P(1))^2)
        all_weight_params(1, 7) = 1.0d0
        all_weight_params(2, 7) = 0.0d0
        
        ! Scheme 8: W=1.0 if Fo<=P(1); W=P(1)/Fo if Fo>P(1)
        all_weight_params(1, 8) = 1.0d0
        
        ! Scheme 9 & 10: no parameters
        
        ! Scheme 11: W=(Sin(theta)/Lambda)^P(1)/(P(2)+P(3)*Fo+P(4)*Fo^2)
        all_weight_params(1, 11) = 2.0d0
        all_weight_params(2, 11) = 1.0d0
        all_weight_params(3, 11) = 0.5d0
        all_weight_params(4, 11) = 0.1d0
        
        ! Scheme 12: W=1/(Sigma(Fo))^2+P(1)*Fo+P(2)*Fo^2)
        all_weight_params(1, 12) = 0.1d0
        all_weight_params(2, 12) = 0.05d0
        
        ! Scheme 13: W=Sc/(P(1)+P(2)*Fo+P(3)*Fo^2+P(4)*(Sin(Th)/L))
        all_weight_params(1, 13) = 1.0d0
        all_weight_params(2, 13) = 0.5d0
        all_weight_params(3, 13) = 0.1d0
        all_weight_params(4, 13) = 0.05d0
        
        ! Scheme 14: W=Sc/(P(1)+P(2)*Fo+P(3)*Fo^2)
        all_weight_params(1, 14) = 1.0d0
        all_weight_params(2, 14) = 0.5d0
        all_weight_params(3, 14) = 0.1d0
        
        ! Scheme 15: W=Sc/(P(1)+P(2)*(Sin(Th)/L)+P(3)*(Sin(Th)/L)^2)
        all_weight_params(1, 15) = 1.0d0
        all_weight_params(2, 15) = 0.5d0
        all_weight_params(3, 15) = 0.1d0
        
        ! Scheme 16: no parameters for now
        
        ! Scheme 17: W=Sc*(Sin(Th)/L)^P(3)/(P(1)*Sigma(Fo))^2+P(2)*Fo^2)
        all_weight_params(1, 17) = 1.0d0
        all_weight_params(2, 17) = 0.1d0
        all_weight_params(3, 17) = 2.0d0
        
        ! Scheme 18: W=Sc/((P(1)+P(2)*Fo+P(3)*Fo^2+P(4)*(Sin(Th)/L)^P(5))*(Sigma(Fo))^2)
        all_weight_params(1, 18) = 1.0d0
        all_weight_params(2, 18) = 0.5d0
        all_weight_params(3, 18) = 0.1d0
        all_weight_params(4, 18) = 0.05d0
        all_weight_params(5, 18) = 2.0d0
        
        ! Number of atoms in the structure
        num_atoms = 25
        
        ! Set error code (0 = success, non-zero = error)
        ier = 0
        
        print *, "Fortran: Returning initial LSQ parameters"
        
    end subroutine lsq_get_parameters
    
    ! Subroutine to execute LSQ calculation with given parameters
    subroutine lsq_execute(refinement_type, damping_factor, reflections_cutoff, &
                           num_cycles, weighting_scheme, weight_params, &
                           refine_weight, num_atoms, fix_xyz, fix_b, fix_occ, &
                           set_isotropic) bind(C, name="lsq_execute")
        integer(c_int), intent(in), value :: refinement_type
        real(c_double), intent(in), value :: damping_factor
        integer(c_int), intent(in), value :: reflections_cutoff
        integer(c_int), intent(in), value :: num_cycles
        integer(c_int), intent(in), value :: weighting_scheme
        real(c_double), intent(in) :: weight_params(10)
        integer(c_int), intent(in), value :: refine_weight
        integer(c_int), intent(in), value :: num_atoms
        integer(c_int), intent(in) :: fix_xyz(*)
        integer(c_int), intent(in) :: fix_b(*)
        integer(c_int), intent(in) :: fix_occ(*)
        integer(c_int), intent(in) :: set_isotropic(*)
        
        character(len=20) :: ref_type_str
        integer :: i
        
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
        print *, "Atoms Configuration:"
        print *, "Number of Atoms: ", num_atoms
        print *, ""
        
        ! Print all atoms with their settings
        do i = 1, num_atoms
            print '(A,I4,A,I1,A,I1,A,I1,A,I1)', "  Atom ", i, &
                ": FixXYZ=", fix_xyz(i), ", FixB=", fix_b(i), &
                ", FixOcc=", fix_occ(i), ", SetIsotropic=", set_isotropic(i)
        end do
        
        print *, ""
        print *, "Summary of selected checkboxes:"
        print *, "  Fix XYZ       : ", count(fix_xyz(1:num_atoms) /= 0), " atoms"
        print *, "  Fix B         : ", count(fix_b(1:num_atoms) /= 0), " atoms"
        print *, "  Fix Occ.      : ", count(fix_occ(1:num_atoms) /= 0), " atoms"
        print *, "  Set Isotropic : ", count(set_isotropic(1:num_atoms) /= 0), " atoms"
        print *, "========================================"
        print *, "Calculation completed successfully!"
        print *, "========================================"
        
    end subroutine lsq_execute
    
    ! Subroutine to get atom data
    subroutine lsq_get_atoms(num_atoms, atom_names, fix_xyz, fix_b, fix_occ, &
                             set_isotropic, ier) bind(C, name="lsq_get_atoms")
        integer(c_int), intent(in), value :: num_atoms
        type(c_ptr), dimension(num_atoms), intent(out) :: atom_names
        integer(c_int), intent(out) :: fix_xyz(*)
        integer(c_int), intent(out) :: fix_b(*)
        integer(c_int), intent(out) :: fix_occ(*)
        integer(c_int), intent(out) :: set_isotropic(*)
        integer(c_int), intent(out) :: ier
        
        integer :: i
        character(len=20, kind=c_char), pointer :: temp_name
        
        ! Initialize arrays
        do i = 1, num_atoms
            ! Allocate and create example atom names (C, N, O, etc.)
            allocate(temp_name)
            
            select case(mod(i-1, 5))
                case(0)
                    temp_name = "C" // trim(adjustl(str(i))) // c_null_char
                case(1)
                    temp_name = "N" // trim(adjustl(str(i))) // c_null_char
                case(2)
                    temp_name = "O" // trim(adjustl(str(i))) // c_null_char
                case(3)
                    temp_name = "H" // trim(adjustl(str(i))) // c_null_char
                case(4)
                    temp_name = "S" // trim(adjustl(str(i))) // c_null_char
            end select
            
            ! Store the C pointer to the string
            atom_names(i) = c_loc(temp_name)
            
            ! Initialize flags with example values (some atoms fixed, some not)
            fix_xyz(i) = mod(i, 3)          ! Every 3rd atom has fix_xyz = 0
            fix_b(i) = mod(i, 4)            ! Every 4th atom has fix_b = 0
            fix_occ(i) = mod(i, 5)          ! Every 5th atom has fix_occ = 0
            set_isotropic(i) = mod(i, 2)    ! Alternating pattern
        end do
        
        ier = 0
        
        print *, "Fortran: Returning atom data for ", num_atoms, " atoms"
        
    contains
        
        function str(k) result(s)
            integer, intent(in) :: k
            character(len=20) :: s
            write(s, '(I0)') k
        end function str
        
    end subroutine lsq_get_atoms
    
end module lsq_module

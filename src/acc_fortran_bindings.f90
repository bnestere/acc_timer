module acc_timer_m
  use, intrinsic :: iso_c_binding, only : c_int, c_ptr, c_null_ptr, c_associated, c_char, c_null_char
  implicit none
  private
  interface
    function initialize_acc_timer_c(n_threads) result(acc_timer) bind(C, name="initialize_acc_timer")
      import :: c_int, c_ptr
      implicit none

      !Argument list
      integer(c_int), intent(in), value :: n_threads

      !function result
      type(c_ptr) :: acc_timer
    end function initialize_acc_timer_c

    subroutine acc_timer_begin_c(timer_ctx, id) bind(C, name="acc_timer_begin")
      import :: c_ptr, c_char
      implicit none

      !Argument list
      type(c_ptr), intent(in), value :: timer_ctx
      character(len=1, kind=c_char), intent(in) :: id(*)

    end subroutine acc_timer_begin_c

    subroutine acc_timer_end_c(timer_ctx, id) bind(C, name="acc_timer_end")
      import :: c_ptr, c_char
      implicit none

      !Argument list
      type(c_ptr), intent(in), value :: timer_ctx
      character(len=1, kind=c_char), intent(in) :: id(*)
    end subroutine acc_timer_end_c

    subroutine acc_write_c(timer_ctx) bind(C, name="acc_write")
      import :: c_ptr
      implicit none

      !Argument list
      type(c_ptr), intent(in), value :: timer_ctx

    end subroutine acc_write_c

!    subroutine destroy_acc_timer_c(timer_ctx) bind(C, name="destroy_acc_timer")
!      import :: c_ptr
!      implicit none
!
!      !Argument list
!      type(c_ptr), intent(in), value :: timer_ctx
!
!    end subroutine destroy_acc_timer_c
    
  end interface

  ! Hold object
  type(c_ptr), save :: obj = c_null_ptr

  public :: Timer_Init
  public :: Timer_Begin
  public :: Timer_End
  public :: Timer_Write
  public :: Timer_Destroy

  contains
    subroutine Timer_Init(n_threads) 
      ! args
      integer(c_int), intent(in),value :: n_threads
      !integer(c_int), value :: n_threads

      obj = initialize_acc_timer_c(n_threads)
      return
    end subroutine Timer_Init

    subroutine Timer_Begin(id) 
      ! args
      
      character(len=*), intent(in) :: id
      character(len=1, kind=C_CHAR) :: c_str(len_trim(id) + 1)
      integer :: N, i

      ! Converting Fortran string to C string
      N = len_trim(id)
      do i = 1, N
        c_str(i) = id(i:i)
      end do

      c_str(N + 1) = c_null_char

      call acc_timer_begin_c(obj, c_str)

      return
    end subroutine Timer_Begin

    subroutine Timer_End(id) 
      ! args
      
      character(len=*), intent(in) :: id
      character(len=1, kind=C_CHAR) :: c_str(len_trim(id) + 1)
      integer :: N, i

      ! Converting Fortran string to C string
      N = len_trim(id)
      do i = 1, N
        c_str(i) = id(i:i)
      end do

      c_str(N + 1) = c_null_char

      call acc_timer_end_c(obj, c_str)

      return
    end subroutine Timer_End

    subroutine Timer_Write() 
      call acc_write_c(obj)

      return
    end subroutine Timer_Write

    subroutine Timer_Destroy() 
!      call destroy_acc_timer_c(obj)

      return
    end subroutine Timer_Destroy
end module

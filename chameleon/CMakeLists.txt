###
#
# @file CMakeLists.txt
#
# @copyright 2009-2014 The University of Tennessee and The University of
#                      Tennessee Research Foundation. All rights reserved.
# @copyright 2012-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                      Univ. Bordeaux. All rights reserved.
#
###
#
#  @project CHAMELEON
#  CHAMELEON is a software package provided by:
#     Inria Bordeaux - Sud-Ouest,
#     Univ. of Tennessee,
#     King Abdullah Univesity of Science and Technology
#     Univ. of California Berkeley,
#     Univ. of Colorado Denver.
#
# @version 1.1.0
#  @author Cedric Castagnede
#  @author Emmanuel Agullo
#  @author Mathieu Faverge
#  @author Florent Pruvost
#  @author Nathalie Furmento
#  @author Philippe Virouleau
#  @author Raphael Boucherie
#  @author Samuel Thibault
#  @date 2021-02-22
#
###
cmake_minimum_required(VERSION 3.3)
project(CHAMELEON Fortran C CXX)

# directly make an error if in-source build
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
   message(FATAL_ERROR "In-source builds are not allowed.\n"
   "Please create a build directory first and execute cmake configuration from "
   "this directory. Example: mkdir build && cd build && cmake ..")
endif()

# set project version number
set(CHAMELEON_VERSION_MAJOR 1)
set(CHAMELEON_VERSION_MINOR 1)
set(CHAMELEON_VERSION_MICRO 0)
set(CHAMELEON_VERSION "${CHAMELEON_VERSION_MAJOR}.${CHAMELEON_VERSION_MINOR}.${CHAMELEON_VERSION_MICRO}")

set(CHAMELEON_CMAKE_DIR "" CACHE PATH "Directory of CHAMELEON CMake modules, can be external to the project")

# Add extra cmake module path and initialize morse cmake modules
# --------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)
if(CHAMELEON_CMAKE_DIR)
    list(APPEND CMAKE_MODULE_PATH "${CHAMELEON_CMAKE_DIR}/cmake_modules/morse_cmake/modules")
    set(CHAMELEON_CMAKE_MODULE_PATH ${CHAMELEON_CMAKE_DIR}/cmake_modules/morse_cmake/modules )
elseif(EXISTS "${CMAKE_SOURCE_DIR}/cmake_modules/morse_cmake")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/morse_cmake/modules")
    set(CHAMELEON_CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules/morse_cmake/modules )
else()
  message(FATAL_ERROR "Chameleon CMake system relies on morse_cmake modules developed here: "
    "https://gitlab.inria.fr/solverstack/morse_cmake. Please set CHAMELEON_CMAKE_DIR to this source "
    "directory.")
endif()

include(MorseInit)
include(GenPkgConfig)

# Ensure that the fortran compiler and c compiler specified are compatible
# ------------------------------------------------------------------------
include(FortranCInterface)
FortranCInterface_VERIFY()
FortranCInterface_HEADER(${CMAKE_CURRENT_BINARY_DIR}/include/chameleon/mangling.h
                         MACRO_NAMESPACE "CHAMELEON_")

#############################################
#                                           #
#        Compilation of CHAMELEON           #
#                                           #
#############################################


###############################################################################
# Parameters/Options #
######################

# For fPIC when build static
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set the RPATH config
# --------------------

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# Misc options
# ------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Define precision supported by CHAMELEON
# -----------------------------------------
set( RP_CHAMELEON_DICTIONNARY ${CMAKE_SOURCE_DIR}/cmake_modules/local_subs.py )
set( RP_CHAMELEON_PRECISIONS  "s;d;c;z" )
include(RulesPrecisions)

# Options to select the runtime
# -----------------------------

# Create a list of possible runtime
set(CHAMELEON_SCHED_list PARSEC STARPU QUARK OPENMP
  CACHE INTERNAL "List of available runtimes" )

set( CHAMELEON_SCHED_PARSEC OFF CACHE INTERNAL
  "Enable PaRSEC scheduler as the default runtime
    (Conflict with other CHAMELEON_SCHED_* options)")
set(CHAMELEON_SCHED_STARPU OFF CACHE INTERNAL
    "Enable StarPU scheduler as the default runtime
    (Conflict with other CHAMELEON_SCHED_* options)")
set(CHAMELEON_SCHED_QUARK OFF CACHE INTERNAL
  "Enable Quark scheduler as the default runtime
    (Conflict with other CHAMELEON_SCHED_* options)")
set(CHAMELEON_SCHED_OPENMP OFF CACHE INTERNAL
  "Enable OpenMP scheduler as the default runtime
    (Conflict with other CHAMELEON_SCHED_* options)")

set( CHAMELEON_SCHED STARPU
  CACHE STRING "Choose the chameleon internal runtime from ${CHAMELEON_SCHED_list}")
set_property(CACHE CHAMELEON_SCHED PROPERTY STRINGS ${CHAMELEON_SCHED_list})

if ( NOT ${CHAMELEON_SCHED} IN_LIST CHAMELEON_SCHED_list )
  message(FATAL_ERROR "CHAMELEON_SCHED must be one of ${CHAMELEON_SCHED_list} (${CHAMELEON_SCHED} is incorrect)" )
endif()

set( CHAMELEON_SCHED_${CHAMELEON_SCHED} ON )

# Set default to StarPU if nothing specific is required by the user
message("-- ${BoldGreen}CHAMELEON_SCHED is set to use ${CHAMELEON_SCHED} runtime" )

# Check that one, and only one, SCHED option is set to ON
# count number of runtime sets to ON
math(EXPR number_of_active_runtime 0)
foreach (runtime ${CHAMELEON_SCHED_list})
    if (CHAMELEON_SCHED_${runtime})
        math(EXPR number_of_active_runtime "${number_of_active_runtime}+1")
    endif()
endforeach()
if (NOT number_of_active_runtime STREQUAL 1)
    message(FATAL_ERROR
            "Number of active runtime is ${number_of_active_runtime}, "
            "the user should activate one (and only one) runtime. ")
endif()

# Use intermediate variable since cmake_dependent_option doesn't have OR conditions
set(CHAMELEON_ENABLE_MPI OFF CACHE INTERNAL "Tells if MPI might be supported by the runtime")
if ( CHAMELEON_SCHED_PARSEC OR CHAMELEON_SCHED_STARPU )
     set(CHAMELEON_ENABLE_MPI ON FORCE)
endif()

# Use intermediate variable since cmake_dependent_option doesn't have OR conditions
set(CHAMELEON_ENABLE_CUDA OFF CACHE INTERNAL "Tells if CUDA might be supported by the runtime")
if ( CHAMELEON_SCHED_PARSEC OR CHAMELEON_SCHED_STARPU )
     set(CHAMELEON_ENABLE_CUDA ON FORCE)
endif()

# Additional options
# ------------------

# Enable the distributed interface (allowed only when StarPU or PaRSEC is enabled)
# TODO: Default should be changed to ON/OFF when it will be ok
cmake_dependent_option(CHAMELEON_USE_MPI
                    "Enable distributed memory through MPI" OFF
                    "CHAMELEON_ENABLE_MPI" OFF)
if (CHAMELEON_ENABLE_MPI AND NOT CHAMELEON_USE_MPI)
    message("-- ${BoldGreen}CHAMELEON_USE_MPI is set to OFF, turn it ON to use MPI (unsupported by Quark)${ColourReset}")
endif()

cmake_dependent_option(CHAMELEON_USE_MIGRATE
  "This options enables the data migration in QR algorithms" ON
  "CHAMELEON_USE_MPI" OFF)

cmake_dependent_option(CHAMELEON_USE_MPI_DATATYPES
  "Enable MPI datatypes whenever supported by the runtime" ON
  "CHAMELEON_USE_MPI" OFF)

# Enable Cuda kernels if StarPU (only if StarPU or PaRSEC runtime is enabled)
# TODO: Default should be changed to ON/OFF when it will be ok
cmake_dependent_option(CHAMELEON_USE_CUDA
                    "Enable CUDA kernels" OFF
                    "CHAMELEON_ENABLE_CUDA" OFF)
if (CHAMELEON_ENABLE_CUDA AND NOT CHAMELEON_USE_CUDA)
    message("-- ${BoldGreen}CHAMELEON_USE_CUDA is set to OFF, turn it ON to use CUDA (unsupported by Quark)${ColourReset}")
endif()

option(CHAMELEON_RUNTIME_SYNC "Enable synchronous task submission when available to debug the code without parallelism" OFF)
if (CHAMELEON_RUNTIME_SYNC)
    message("-- ${BoldGreen}CHAMELEON_RUNTIME_SYNC is set to ON, turn it OFF to avoid synchronisation in the tasks submission${ColourReset}")
endif()

# Options to enable/disable doc, examples, and testings
# -----------------------------------------------------
option(CHAMELEON_ENABLE_DOC      "Enable documentation build"  OFF)
if (CHAMELEON_ENABLE_DOC)
    message("-- ${BoldGreen}CHAMELEON_ENABLE_DOC is set to ON, turn it OFF to avoid building docs${ColourReset}")
endif()
option(CHAMELEON_ENABLE_EXAMPLE  "Enable examples build"       ON)
if (CHAMELEON_ENABLE_EXAMPLE)
    message("-- ${BoldGreen}CHAMELEON_ENABLE_EXAMPLE is set to ON, turn it OFF to avoid building examples${ColourReset}")
endif()
option(CHAMELEON_ENABLE_TESTING  "Enable testings build"       ON)
if (CHAMELEON_ENABLE_TESTING)
    message("-- ${BoldGreen}CHAMELEON_ENABLE_TESTING is set to ON, turn it OFF to avoid building testing${ColourReset}")
endif()

# Option to activate or not simulation mode (use Simgrid through StarPU)
# ----------------------------------------------------------------------
if(CHAMELEON_SCHED_STARPU)
    option(CHAMELEON_SIMULATION "Enable simulation mode using Simgrid through StarPU" OFF)
    if (NOT CHAMELEON_SIMULATION)
        message("-- ${BoldGreen}CHAMELEON_SIMULATION is set to OFF, turn it ON to use"
            " SIMULATION mode (only with StarPU compiled with SimGrid)${ColourReset}")
    endif()
    option(CHAMELEON_ENABLE_PRUNING_STATS "Enable pruning statistics" OFF)
    if (NOT CHAMELEON_ENABLE_PRUNING_STATS)
        message("-- ${BoldGreen}CHAMELEON_ENABLE_PRUNING_STATS is set to OFF, turn it ON to build pruning statistics${ColourReset}")
    endif()
    cmake_dependent_option(CHAMELEON_SIMULATION_EXTENDED
      "Enable non GPU kernels to be simulated on GPU" OFF
      "CHAMELEON_SIMULATION" OFF)
endif()

# Initially we need to generate files for different precisions
# TODO: use this option to avoid generate multiple precisions each time we launch cmake
#option(CHAMELEON_GEN_PREC "Generate source files precisions" ON)

if ( CHAMELEON_SCHED_OPENMP OR CHAMELEON_SCHED_STARPU )
  if ( CHAMELEON_USE_CUDA )
    set( CHAMELEON_COPY_DIAG ON )
  else()
    option(CHAMELEON_COPY_DIAG
      "This options enables the duplication of the diagonal tiles in some algorithm to avoid anti-dependencies on lower/upper triangular parts (Might be useful to StarPU and OpenMP)" ON)
  endif()
endif()
if ( CHAMELEON_SCHED_QUARK )
  # No need for those extra diagonale tiles
  set( CHAMELEON_COPY_DIAG OFF )
endif()
if ( CHAMELEON_SCHED_PARSEC )
  # Actual DIAG can not be used with PaRSEC
  set( CHAMELEON_COPY_DIAG OFF )
endif()
mark_as_advanced(CHAMELEON_COPY_DIAG)
#------------------------------------------------------------------------------

###############################################################################
# Build dependency HQR library #
################################
add_subdirectory(hqr)

###############################################################################
# Look for dependencies #
#########################

# Add math library (libm), searched with MORSE module FindM.cmake
# it normally exists on all common systems provided with a C compiler
find_package(M REQUIRED)

# If simulation we don't enter in kernel functions so that we don't need to link
# with concerned libraries
if(NOT CHAMELEON_SIMULATION)

    # CHAMELEON depends on CBLAS
    #---------------------------
    set( CBLAS_MT FALSE )
    find_package(CBLAS REQUIRED)

    # CHAMELEON depends on LAPACKE
    #-----------------------------
    set( LAPACKE_MT FALSE )
    find_package(LAPACKE REQUIRED)

    # CHAMELEON depends on CUDA/CUBLAS
    #----------------------------------
    if (CHAMELEON_USE_CUDA)

        # When our cmake_minimum version will be 3.17 we will be able to use this
        # far better module: FindCUDAToolkit
        # https://cmake.org/cmake/help/latest/module/FindCUDAToolkit.html#module:FindCUDAToolkit

        find_package(CUDA REQUIRED)

        if (CUDA_FOUND)
            message("-- ${Blue}Add definition CHAMELEON_USE_CUDA"
            " - Activate CUDA in Chameleon${ColourReset}")
            # create imported target because not provided with old cmake
            add_library(CUDA::CUDA INTERFACE IMPORTED)
            add_library(CUDA::CUBLAS INTERFACE IMPORTED)
            if(CUDA_VERSION VERSION_LESS "3.0")
                set(CUDA_HOST_COMPILATION_CPP OFF)
            endif(CUDA_VERSION VERSION_LESS "3.0")
            set(CUDA_BUILD_EMULATION OFF)
            if (CUDA_INCLUDE_DIRS)
                set_target_properties(CUDA::CUDA PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIRS}")
            else()
                message(WARNING "CHAMELEON_USE_CUDA requires"
                "\n   CUDA_INCLUDE_DIRS to be found. Be sure you have"
                "\n   cuda headers with your distribution of CUDA.")
            endif()
            if (CUDA_LIBRARIES)
                set_target_properties(CUDA::CUDA PROPERTIES INTERFACE_LINK_LIBRARIES "${CUDA_LIBRARIES}")
                set(CMAKE_REQUIRED_INCLUDES  "${CUDA_INCLUDE_DIRS}")
                set(CMAKE_REQUIRED_LIBRARIES "${CUDA_LIBRARIES}")
                if(CUDA_VERSION VERSION_LESS "4.0")
                    set(CUDA_HAVE_PEER_DEVICE_MEMORY_ACCESS 0)
                else()
                    check_function_exists(cuDeviceCanAccessPeer CUDA_HAVE_PEER_DEVICE_MEMORY_ACCESS)
                endif()
                unset(CMAKE_REQUIRED_INCLUDES)
                unset(CMAKE_REQUIRED_LIBRARIES)
                # Add cublas if found
                if (CUDA_CUBLAS_LIBRARIES)
                    set_target_properties(CUDA::CUBLAS PROPERTIES INTERFACE_LINK_LIBRARIES "${CUDA_CUBLAS_LIBRARIES}")
                    target_link_libraries(CUDA::CUBLAS INTERFACE CUDA::CUDA)
                    message("-- ${Blue}Add definition CHAMELEON_USE_CUBLAS"
                    " - Use GPU kernels from cuBLAS${ColourReset}")
                else()
                    message(FATAL_ERROR "CHAMELEON_USE_CUDA requires"
                    "\n   CUDA_CUBLAS_LIBRARIES to be found. Be sure you have"
                    "\n   libcublas with your distribution of CUDA.")
                endif()
            else()
                message(FATAL_ERROR "CHAMELEON_USE_CUDA requires"
                "\n   CUDA_LIBRARIES to be found. Be sure you have"
                "\n   libcuda with your distribution of CUDA.")
            endif()
        endif (CUDA_FOUND)

    endif(CHAMELEON_USE_CUDA)

    # CHAMELEON depends on MPI
    #-------------------------
    if (CHAMELEON_USE_MPI)

        # allows to use an external mpi compilation by setting compilers with
        # -DMPI_C_COMPILER=path/to/mpicc -DMPI_Fortran_COMPILER=path/to/mpif90
        # at cmake configure
        if(NOT MPI_C_COMPILER)
            set(MPI_C_COMPILER mpicc)
        endif()
        find_package(MPI REQUIRED)

        if (MPI_C_FOUND)
            message("-- ${Blue}Add definition CHAMELEON_USE_MPI"
            " - Activate MPI in Chameleon${ColourReset}")
        endif (MPI_C_FOUND)

    endif (CHAMELEON_USE_MPI)

else (NOT CHAMELEON_SIMULATION)

    message("-- ${BoldBlue}Simulation mode of CHAMELEON is activated (CHAMELEON_SIMULATION=ON)."
    "\n   This mode allows you to simulate execution of algorithms with StarPU compiled with SimGrid."
    "\n   To do so, we provide some perfmodels in the simucore/perfmodels/ directory of CHAMELEON sources."
    "\n   The resulting directory where to find perfmodels is: ${CMAKE_CURRENT_SOURCE_DIR}/simucore/perfmodels."
    "\n   To use these perfmodels, please set your STARPU_HOME environment variable to ${CMAKE_CURRENT_SOURCE_DIR}/simucore/perfmodels."
    "\n   Finally, you need to set your STARPU_HOSTNAME environment variable to the name of the machine to simulate."
    "\n   For example: STARPU_HOSTNAME=mirage."
    "\n   Note that only POTRF kernels (simple and double precision) on mirage machine (block sizes of 320 or 960) and sirocco (block sizes of 80, 440, 960, 1440, 1920) are available for now."
    "\n   Database of models is subject to change, it should be enrich in a near future.${ColourReset}")

    message("-- ${Blue}Add definition CHAMELEON_SIMULATION"
    " - Activate simulation mode in Chameleon (to use with StarPU+SimGrid)${ColourReset}")

    add_subdirectory(simucore)

    # Guard against mis-used simulation mode
    if(NOT DEFINED CHAMELEON_USE_CUDA)
        message(FATAL_ERROR "${BoldBlue}"
        "In simulation mode CHAMELEON_USE_CUDA should be set to"
        "   ensure that the user is aware of the version to be used. If Chameleon's"
        "   kernels are available for NVIDIA CUDA GPUs and if the according"
        "   perfmodels are available in STARPU_HOME then use CHAMELEON_USE_CUDA=ON"
        "   else set CHAMELEON_USE_CUDA=OFF."
        "${ColourReset}")
    endif()

    if (NOT CHAMELEON_SCHED_STARPU)
        message(FATAL_ERROR "Simulation mode of Chameleon only works with"
        "\n   the StarPU runtime. Please use CHAMELEON_SCHED_STARPU=ON.")
    endif()

    if (CHAMELEON_ENABLE_EXAMPLE)
        set(CHAMELEON_ENABLE_EXAMPLE OFF)
        message("-- ${BoldBlue}CHAMELEON_ENABLE_EXAMPLE is set to ON but we turn it OFF.")
    endif ()

    # Simulation mode: we depend on SimGrid
    find_package(SIMGRID REQUIRED)

    # CHAMELEON depends on MPI
    #-------------------------
    if (CHAMELEON_USE_MPI)

        # allows to use an external mpi compilation by setting compilers with
        # -DMPI_C_COMPILER=path/to/mpicc -DMPI_Fortran_COMPILER=path/to/mpif90
        # at cmake configure
        if(NOT MPI_C_COMPILER)
            set(MPI_C_COMPILER "${SIMGRID_PREFIX}/bin/smpicc")
        endif()
        if(NOT MPI_C_LIBRARIES)
            set(MPI_C_LIBRARIES "${SIMGRID_LIBRARIES}")
        endif()
        if(NOT MPI_C_INCLUDE_PATH)
            set(MPI_C_INCLUDE_PATH "${SIMGRID_INCLUDE_DIRS}")
            list(APPEND MPI_C_INCLUDE_PATH "${SIMGRID_INCLUDE_DIRS}/smpi")
        endif()
        if(NOT MPI_CXX_COMPILER)
            set(MPI_CXX_COMPILER "${SIMGRID_PREFIX}/bin/smpicxx")
        endif()
        if(NOT MPI_CXX_LIBRARIES)
            set(MPI_CXX_LIBRARIES "${SIMGRID_LIBRARIES}")
        endif()
        if(NOT MPI_CXX_INCLUDE_PATH)
            set(MPI_CXX_INCLUDE_PATH "${SIMGRID_INCLUDE_DIRS}")
            list(APPEND MPI_CXX_INCLUDE_PATH "${SIMGRID_INCLUDE_DIRS}/smpi")
        endif()
        if(NOT MPI_Fortran_COMPILER)
            set(MPI_Fortran_COMPILER "${SIMGRID_PREFIX}/bin/smpif90")
        endif()
        if(NOT MPI_Fortran_LIBRARIES)
            set(MPI_Fortran_LIBRARIES "${SIMGRID_LIBRARIES}")
        endif()
        if(NOT MPI_Fortran_INCLUDE_PATH)
            set(MPI_Fortran_INCLUDE_PATH "${SIMGRID_INCLUDE_DIRS}")
            list(APPEND MPI_Fortran_INCLUDE_PATH "${SIMGRID_INCLUDE_DIRS}/smpi")
        endif()
        find_package(MPI REQUIRED)

        if (MPI_C_FOUND)
            message("-- ${Blue}Add definition CHAMELEON_USE_MPI"
            " - Activate MPI in Chameleon${ColourReset}")
        endif (MPI_C_FOUND)

    endif (CHAMELEON_USE_MPI)

endif(NOT CHAMELEON_SIMULATION)

# getopt
unset(CMAKE_REQUIRED_LIBRARIES)
check_include_files(getopt.h CHAMELEON_HAVE_GETOPT_H)
if (CHAMELEON_HAVE_GETOPT_H)
  check_function_exists(getopt_long CHAMELEON_HAVE_GETOPT_LONG)
endif()

# Add option to exploit cublas API v2
# -----------------------------------
cmake_dependent_option(CHAMELEON_USE_CUBLAS_V2
  "Enable cublas API v2" ON
  "CHAMELEON_USE_CUDA;CHAMELEON_SCHED_STARPU" OFF)

# Fix a problem on Mac OS X when building shared libraries
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
endif()

# Check for the subdirectories
# ----------------------------

set(CHAMELEON_SOURCES_TARGETS "" CACHE INTERNAL "List of targets of sources")

if (NOT CHAMELEON_SIMULATION)
  ###############################################################################
  # Coreblas library (kernels for CPUs) #
  #######################################
  add_subdirectory(coreblas)

  ###############################################################################
  # Cudablas library (kernels for CUDAs) #
  ########################################
  if(CHAMELEON_USE_CUDA)
    add_subdirectory(cudablas)
  endif()

endif()

###############################################################################
# Runtimes libraries #
######################
add_subdirectory(runtime)

###############################################################################
# Main library #
################
add_subdirectory(include)
add_subdirectory(control)
add_subdirectory(compute)

###############################################################################
# Executables and Tests #
#########################
enable_testing()
include(CTest)

# Necessary to compile C executables linked with ifort
if(${CMAKE_Fortran_COMPILER_ID} STREQUAL "Intel")
  message(STATUS "Add -nofor_main to the Fortran linker (Intel compiler)")
  set(CMAKE_Fortran_FLAGS  "${CMAKE_Fortran_FLAGS} -nofor_main")
endif()

# Testing executables
if(CHAMELEON_ENABLE_EXAMPLE AND NOT CHAMELEON_SIMULATION)
  add_subdirectory(example)
endif(CHAMELEON_ENABLE_EXAMPLE AND NOT CHAMELEON_SIMULATION)

# Testing executables
if(CHAMELEON_ENABLE_TESTING)
  add_subdirectory(testing)
endif(CHAMELEON_ENABLE_TESTING)
#------------------------------------------------------------------------------

# Define a target which gathers all targets of sources
# useful for generating the doc without compiling the binaries (objects, libraries)
add_custom_target(chameleon_all_sources ALL DEPENDS ${CHAMELEON_SOURCES_TARGETS})

###############################################################################
# Documentation #
#################

if(CHAMELEON_ENABLE_DOC)
  add_subdirectory(doc)
  if (DOXYGEN_EXECUTABLE AND EMACS_COMPILER AND TEX_COMPILER)
    add_custom_target(doc ALL DEPENDS doxygen-out doc-html-users_guide doc-pdf-users_guide)
  endif()
endif()
#------------------------------------------------------------------------------


###############################################################################
# Make cmake include starpu_simgrid_wrap.h (which #define main and thus makes #
# link tests fail) only after tests.                                          #
###############################################################################
if( CHAMELEON_SCHED_STARPU )
  if ( STARPU_FOUND )
    if (CHAMELEON_SIMULATION)
      list(APPEND CMAKE_REQUIRED_FLAGS "-include" "starpu_simgrid_wrap.h")
    endif()
  endif()
endif()

###############################################################################
# Export targets #
##################

# see https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

set(BIN_INSTALL_DIR "bin/" CACHE STRING "where to install executables relative to prefix" )
set(INC_INSTALL_DIR "include/" CACHE STRING "where to install headers relative to prefix" )
set(LIB_INSTALL_DIR "lib/" CACHE STRING "where to install libraries relative to prefix" )

configure_package_config_file(cmake_modules/CHAMELEONConfig.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/CHAMELEONConfig.cmake
                              INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/chameleon
                              PATH_VARS BIN_INSTALL_DIR INC_INSTALL_DIR LIB_INSTALL_DIR)
write_basic_package_version_file(CHAMELEONConfigVersion.cmake
                                 VERSION ${CHAMELEON_VERSION}
                                 COMPATIBILITY AnyNewerVersion)

# Install config files
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CHAMELEONConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/CHAMELEONConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/chameleon)

# need MORSE Find modules: necessary files must be distributed in the install path
set(morse_dependencies "M;BLASEXT;LAPACKEXT;CBLAS;LAPACKE;HWLOC;PARSEC;QUARK;STARPU")
morse_install_finds(morse_dependencies ${LIB_INSTALL_DIR}/cmake/chameleon/find)

###############################################################################
# Config files (pkg_config) #
#############################

# Create .pc file
# ---------------
generate_pkgconfig_file()
#------------------------------------------------------------------------------

###############################################################################
# Release source #
##################
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME "chameleon")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.org)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Chameleon is a C library providing parallel algorithms to perform BLAS/LAPACK "
                                      " operations exploiting fully modern architectures.")
set(CHAMELEON_VERSION_PATCH ${CHAMELEON_VERSION_MICRO})
set(CPACK_PACKAGE_VERSION "${CHAMELEON_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_VENDOR "Inria")
set(CPACK_PACKAGE_CONTACT "morse-devel@lists.gforge.inria.fr")
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.org)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.txt)
set(CPACK_SOURCE_IGNORE_FILES "/build*;/.*")
include(CPack)

###############################################################################
# Print Options #
#################
include(PrintOpts)

if (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local" OR CMAKE_INSTALL_PREFIX STREQUAL "C:/Program Files")
  message("-- ${Yellow}Your CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX} which is a default system path."
  " You may want to change it: set the CMAKE_INSTALL_PREFIX variable to do so${ColourReset}")
else()
  message("-- ${Yellow}Your CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}${ColourReset}")
endif()

###
### END CMakeLists.txt
###

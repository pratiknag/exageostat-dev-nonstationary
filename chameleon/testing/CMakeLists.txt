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
#  @author Lucas Barros de Assis
#  @author Florent Pruvost
#  @date 2021-01-04
#
###

# Generate chameleon auxiliary testing sources for all possible precisions
# --------------------------------------------------------------------
set(TESTING_HDRS_GENERATED "")
set(ZHDR
  testing_zcheck.h
 )

precisions_rules_py(TESTING_HDRS_GENERATED "${ZHDR}"
    PRECISIONS "s;d;c;z;ds;zc" )

add_custom_target(testing_include ALL SOURCES ${TESTING_HDRS_GENERATED})
set(CHAMELEON_SOURCES_TARGETS "${CHAMELEON_SOURCES_TARGETS};testing_include" CACHE INTERNAL "List of targets of sources")

# Generate chameleon testing sources for all possible precisions
# ----------------------------------------------------------
set(ZSRC
  chameleon_ztesting.c
  testing_zcheck.c
  ##################
  # LAPACK
  ##################
  #testing_zlaset.c
  testing_zlacpy.c
  testing_zlange.c
  testing_zlanhe.c
  testing_zlansy.c
  testing_zlantr.c
  testing_zgeadd.c
  testing_ztradd.c
  testing_zlascal.c
  testing_zgemm.c
  testing_zhemm.c
  testing_zherk.c
  testing_zher2k.c
  testing_zsymm.c
  testing_zsyrk.c
  testing_zsyr2k.c
  testing_ztrmm.c
  testing_ztrsm.c
  testing_zpotrf.c
  testing_zpotrs.c
  testing_zposv.c
  testing_ztrtri.c
  testing_zlauum.c
  testing_zpotri.c
  testing_zsytrf.c
  testing_zsytrs.c
  testing_zsysv.c
  testing_zgenm2.c
  testing_zgetrf.c
  testing_zgetrs.c
  testing_zgesv.c
  testing_zgeqrf.c
  testing_zungqr.c
  testing_zunmqr.c
  testing_zgelqf.c
  testing_zunglq.c
  testing_zunmlq.c
  # testing_zgeqrs.c
  # testing_zgelqs.c
  testing_zgels.c
  testing_zgeqrf_hqr.c
  testing_zungqr_hqr.c
  testing_zunmqr_hqr.c
  testing_zgelqf_hqr.c
  testing_zunglq_hqr.c
  testing_zunmlq_hqr.c
  # testing_zgeqrs_hqr.c
  # testing_zgelqs_hqr.c
  testing_zgels_hqr.c
  testing_zgepdf_qdwh.c
  testing_zgepdf_qr.c
  testing_zplrnk.c
  )


# Create one testing per precision with all files
# -----------------------------------------------
foreach(_precision ${CHAMELEON_PRECISION} )

  precisions_rules_py(${_precision}SRC_GENERATED "${ZSRC}"
    PRECISIONS "${_precision}" )

  set( __target_name chameleon_${_precision}testing )
  add_executable( ${__target_name}
    ${${_precision}SRC_GENERATED}
    values.c
    run_list.c
    parameters.c
    )
  add_dependencies(${__target_name}
    chameleon_include
    control_include
    testing_include
    )
  if(NOT CHAMELEON_SIMULATION)
    add_dependencies(${__target_name}
      coreblas_include
      )
  endif(NOT CHAMELEON_SIMULATION)

  target_include_directories(${__target_name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )
  # testing_zcheck.c includes additional chameleon headers
  # coreblas/include/coreblas/cblas.h, coreblas/coreblas_z.h, control/common.h, ...
  target_include_directories(${__target_name} PRIVATE
    ${CHAMELEON_SOURCE_DIR}
    ${CHAMELEON_BINARY_DIR}
    ${CHAMELEON_SOURCE_DIR}/coreblas/include
    ${CHAMELEON_BINARY_DIR}/coreblas/include
    )

  target_link_libraries(${__target_name} PRIVATE chameleon)
  if(NOT CHAMELEON_SIMULATION)
    target_link_libraries(${__target_name} PRIVATE
      MORSE::LAPACKE
      MORSE::CBLAS)
  endif()
  target_link_libraries(${__target_name} PRIVATE MORSE::M)

  install(TARGETS ${__target_name}
    DESTINATION bin/ )

endforeach()

# Force generation of sources
# ---------------------------
set(TESTING_SRCS)
foreach(_precision ${CHAMELEON_PRECISION})
  list(APPEND TESTING_SRCS ${${_precision}SRC_GENERATED})
endforeach()
add_custom_target(testing_sources ALL SOURCES ${TESTING_SRCS})
set(CHAMELEON_SOURCES_TARGETS "${CHAMELEON_SOURCES_TARGETS};testing_sources" CACHE INTERNAL "List of targets of sources")

#-------- Tests ---------
include(CTestLists.cmake)

# copy input files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

###
### END CMakeLists.txt
###

# @copyright (c) 2017 King Abdullah University of Science and
#                      Technology (KAUST). All rights reserved.
#
# STARS-H is a software package, provided by King Abdullah
#             University of Science and Technology (KAUST)
#
# @file src/applications/electrodynamics/CMakeLists.txt
# @version 0.3.0
# @author Aleksandr Mikhalev
# @date 2017-11-07


EXECUTE_PROCESS(COMMAND "python"
    "../misc_scripts/code_generation/applications/particles/kernel_nd.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel_sin.c"
    "${CMAKE_CURRENT_BINARY_DIR}"
    OUTPUT_VARIABLE generated_files)

EXECUTE_PROCESS(COMMAND "python"
    "../misc_scripts/code_generation/applications/particles/kernel_nd.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel_cos.c"
    "${CMAKE_CURRENT_BINARY_DIR}"
    OUTPUT_VARIABLE generated_files2)
list(APPEND generated_files "${generated_files2}")

#message("${generated_files}")
set(STARSH_SRC
    ${generated_files}
    ${STARSH_SRC} PARENT_SCOPE)

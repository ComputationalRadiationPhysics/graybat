# CMake generated Testfile for 
# Source directory: /home/erik/projects/graybat
# Build directory: /home/erik/projects/graybat/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(graybat_check_build "/usr/bin/cmake" "--build" "/home/erik/projects/graybat/build" "--target" "check")
add_test(graybat_example_build "/usr/bin/cmake" "--build" "/home/erik/projects/graybat/build" "--target" "example")
add_test(graybat_test_run "mpiexec" "--allow-run-as-root" "-n" "2" "check")
set_tests_properties(graybat_test_run PROPERTIES  DEPENDS "graybat_check_build")
add_test(graybat_gol_run "mpiexec" "--allow-run-as-root" "gol" "90" "4")
set_tests_properties(graybat_gol_run PROPERTIES  DEPENDS "graybat_example_build")

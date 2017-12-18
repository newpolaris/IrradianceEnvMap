# CMake generated Testfile for 
# Source directory: C:/Users/newpolaris/Projects/Irradiance/external/FreeImage
# Build directory: C:/Users/newpolaris/Projects/Irradiance/external/FreeImage
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(FreeImageTest "C:/Users/newpolaris/Projects/Irradiance/external/FreeImage/Debug/FreeImageTest.exe")
  set_tests_properties(FreeImageTest PROPERTIES  WORKING_DIRECTORY "C:/Users/newpolaris/Projects/Irradiance/test")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(FreeImageTest "C:/Users/newpolaris/Projects/Irradiance/external/FreeImage/Release/FreeImageTest.exe")
  set_tests_properties(FreeImageTest PROPERTIES  WORKING_DIRECTORY "C:/Users/newpolaris/Projects/Irradiance/test")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(FreeImageTest "C:/Users/newpolaris/Projects/Irradiance/external/FreeImage/MinSizeRel/FreeImageTest.exe")
  set_tests_properties(FreeImageTest PROPERTIES  WORKING_DIRECTORY "C:/Users/newpolaris/Projects/Irradiance/test")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(FreeImageTest "C:/Users/newpolaris/Projects/Irradiance/external/FreeImage/RelWithDebInfo/FreeImageTest.exe")
  set_tests_properties(FreeImageTest PROPERTIES  WORKING_DIRECTORY "C:/Users/newpolaris/Projects/Irradiance/test")
else()
  add_test(FreeImageTest NOT_AVAILABLE)
endif()

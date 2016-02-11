set(LNAME energymon-papi)
set(SOURCES ${LNAME}.c;../src/energymon-util.c)
set(DESCRIPTION "EnergyMon implementation for PAPI")

# Dependencies

find_package(PkgConfig REQUIRED)
pkg_check_modules(PAPI REQUIRED papi)

# Libraries

add_library(${LNAME} ${SOURCES})
target_link_libraries(${LNAME} -L${PAPI_LIBDIR} ${PAPI_LIBRARIES})

if(DEFAULT STREQUAL "papi" OR DEFAULT STREQUAL LNAME)
  BUILD_DEFAULT("${SOURCES}" "-L${PAPI_LIBDIR};${PAPI_LIBRARIES}" "")
  PKG_CONFIG("energymon-default" "${DESCRIPTION}" "${PAPI_LIBRARIES}" "")
  install(TARGETS energymon-default DESTINATION lib)
endif()

# pkg-config

PKG_CONFIG("${LNAME}" "${DESCRIPTION}" "" "")

# Install

install(TARGETS ${LNAME} DESTINATION lib)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${LNAME}.h DESTINATION include/${PROJECT_NAME})

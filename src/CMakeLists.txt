file(GLOB horizonengine_SOURCES *.cpp)

file(GLOB horizonengine_HEADERS *.h)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set(horizonengine_COMPILE
    ${horizonengine_COMPILE} ${horizonengine_SOURCES} ${horizonengine_HEADERS})

if(APPLE)
  list(APPEND horizonengine_COMPILE ${horizonengine_DARWIN_SOURCES})
  # On Macs we must ensure the required frameworks are linked
  find_library(AUDIOUNITS_LIBRARY "AudioUnit")
  find_library(AUDIOTOOLBOX_LIBRARY "AudioToolbox")
  find_library(ACCELERATE_LIBRARY "Accelerate")
  find_library(COCOA_LIBRARY "Cocoa")
  find_library(COREAUDIO_LIBRARY "CoreAudio")
  find_library(CORESERVICES_LIBRARY "CoreServices")

  mark_as_advanced(AUDIOUNITS_LIBRARY AUDIOTOOLBOX_LIBRARY ACCELERATE_LIBRARY
                   COCOA_LIBRARY COREAUDIO_LIBRARY CORESERVICES_LIBRARY)
  list(
    APPEND
    horizonengine_LIBRARIES
    "${AUDIOUNITS_LIBRARY}"
    "${AUDIOTOOLBOX_LIBRARY}"
    "${ACCELERATE_LIBRARY}"
    "${COCOA_LIBRARY}"
    "${COREAUDIO_LIBRARY}"
    "${CORESERVICES_LIBRARY}")

endif()

add_executable(${HORIZONENGINE_EXECUTABLE_NAME} ${horizonengine_COMPILE})

# Qt modules
list(APPEND horizonengine_qt_modules "Network")
foreach(horizonengine_qt_module ${horizonengine_qt_modules})
  message(STATUS "Finding QT Plugin ${horizonengine_qt_module}")
  find_package(Qt5${horizonengine_qt_module} REQUIRED)
  list(APPEND horizonengine_LIBRARIES Qt5::${horizonengine_qt_module})
endforeach()

# LabSound
list(APPEND horizonengine_LIBRARIES LabSound)

# libnyquist
list(APPEND horizonengine_LIBRARIES libnyquist)

# libwavpack
list(APPEND horizonengine_LIBRARIES libwavpack)

set_target_properties(${HORIZONENGINE_EXECUTABLE_NAME} PROPERTIES COMPILE_DEFINITIONS "${horizonengine_DEFINITIONS}")

set_target_properties(${HORIZONENGINE_EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "${horizonengine_LINK_FLAGS}")

foreach(module ${horizonengine_LIBRARIES})
  message(STATUS "Target link module ${module}")
endforeach()

target_link_libraries(${HORIZONENGINE_EXECUTABLE_NAME} ${horizonengine_LIBRARIES})

if(DEFINED HORIZON_EXECUTABLE_NAME)
    message(STATUS "Horizon found in project parent. Setting appropriate target directory...")
    if (APPLE)
        set_target_properties(${HORIZONENGINE_EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${HORIZON_CURRENT_BINARY_DIR}/${HORIZON_EXECUTABLE_NAME}.app/Contents/MacOS)
    else()
        set_target_properties(${HORIZONENGINE_EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${HORIZON_CURRENT_BINARY_DIR}/../)
    endif()
else()
    message(STATUS "Horizon not found. Building standalone...")
endif()

install(TARGETS ${HORIZONENGINE_EXECUTABLE_NAME}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

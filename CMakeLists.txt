cmake_minimum_required(VERSION 3.00)

project(VideoFaceExtractor CXX)
set(EXE VideoFaceExtractor)

set(CMAKE_CXX_STANDARD 20)

#file(COPY ./haarcascade_frontalface_default.xml DESTINATION ${PROJECT_BINARY_DIR})

add_executable (${EXE}
"src/Utils.cpp" "src/Utils.h"
"src/Main.cpp"
"src/FaceDetector.h" "src/FaceDetector.cpp"
"src/ListWriter.h" "src/ListWriter.cpp"
"src/App.h" "src/App.cpp"
src/CNN_Detector.cpp src/CNN_Detector.h src/Extractor.cpp src/Extractor.h)

include_directories(include)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${EXE} PROPERTY CXX_STANDARD 20)
endif()

find_package(OpenMP REQUIRED)
if (OpenMP_FOUND)
	message("OPENMP FOUND")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(facedetection_DIR deps/src/libfacedetection/build)

find_package(facedetection CONFIG)
if (facedetection_FOUND)
	message("Found facedetection")
	target_link_directories(${EXE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/build/lib)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_link_libraries( ${EXE} PUBLIC libfacedetection.a)
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#	target_link_libraries( ${EXE} PUBLIC facedetection.lib)
else()
	target_link_libraries( ${EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/build/lib/facedetection.lib)
endif()

	#target_link_libraries(${EXE} PRIVATE facedetection)
	#message(INFO ${facedetection_INCLUDE_DIRS})
	target_include_directories(${EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/build/include/facedetection)
endif()

find_package(Boost COMPONENTS program_options REQUIRED)
if (Boost_FOUND)
	target_link_libraries(${EXE} PRIVATE Boost::program_options)
	target_include_directories(${EXE} PRIVATE ${Boost_INCLUDE_DIRS})
endif()

find_package(OpenCV CONFIG REQUIRED)
if (OpenCV_FOUND)
	target_link_libraries(${EXE} PRIVATE ${OpenCV_LIBS})
	target_include_directories(${EXE} PRIVATE ${OpenCV_INCLUDE_DIRS})
endif()

#target_link_libraries(${EXE}) # blas-openblas

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	find_package(OpenBLAS CONFIG REQUIRED)
	if (OpenBLAS_FOUND)
		target_link_libraries(${EXE} PRIVATE ${OpenBLAS_LIBRARIES} cblas)
		target_include_directories(${EXE} PRIVATE ${OpenBLAS_INCLUDE_DIRS})
	endif()

	# using GCC
	set(CMAKE_VERBOSE_MAKEFILE 1)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "Bits64_;UNIX;_BOOL;LINUX;FUNCPROTO;_GNU_SOURCE;LINUX_64;REQUIRE_IOSTREAM")
	set(GCC_COMPILE_DEBUG_OPTIONS "${GCC_COMPILE_OPTIONS};-ggdb;-O0")
	set(GCC_COMPILE_RELEASE_OPTIONS "${GCC_COMPILE_OPTIONS};-O3")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:${GCC_COMPILE_DEBUG_OPTIONS}>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:${GCC_COMPILE_RELEASE_OPTIONS}>")

#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
	# using Visual Studio C++
	#target_compile_options(${PROJECT_NAME} PRIVATE "/utf-8")
	#set(MSVC_COMPILE_OPTIONS "${MSVC_COMPILE_OPTIONS} /utf-8")
	set(MSVC_COMPILE_DEBUG_OPTIONS /utf-8 ${MSVC_COMPILE_OPTIONS} /ZI /Od)
	set(MSVC_COMPILE_RELEASE_OPTIONS /utf-8 ${MSVC_COMPILE_OPTIONS} /O2)
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:${MSVC_COMPILE_DEBUG_OPTIONS}>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:${MSVC_COMPILE_RELEASE_OPTIONS}>")
endif()



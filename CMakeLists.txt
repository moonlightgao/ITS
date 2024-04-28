cmake_minimum_required(VERSION 3.10)
#project(MyProject VERSION 1.0)
#project(MyOpenCVProject)
#cmake_minimum_required(VERSION 3.13)
project(ITS)
# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SAMPLES_COMMON_SOURCES "/home/alice/tensorrt_tar/TensorRT-8.6.1.6/samples/common/logger.cpp")



#cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
#project(pt_unet)
set(CMAKE_PREFIX_PATH "/home/alice/project/libtorch/libtorch-shared-with-deps-1.13.0+cu116/libtorch" ${CMAKE_PREFIX_PATH})
find_package(Torch REQUIRED)
#find_package(OpenCV REQUIRED)

#add_executable(pt_unet main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hyperlpr3/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hyperlpr3/lib)
link_directories(/usr/local/ffmpeg/lib/)


#------------------------------------------------------------
# jsoncpp
#------------------------------------------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}/thirdparty/jsoncpp/include
)
aux_source_directory(${CMAKE_SOURCE_DIR}/thirdparty/jsoncpp/src SRC_DIR)


include_directories(
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/CarDectorService
    ${CMAKE_SOURCE_DIR}/src/CarDectorService/TrtDetector
    ${CMAKE_SOURCE_DIR}/src/DrawService
    ${CMAKE_SOURCE_DIR}/src/TrackingService
    ${CMAKE_SOURCE_DIR}/src/TrackingService/TrtTracking
    ${CMAKE_SOURCE_DIR}/src/PlateRecogService
    ${CMAKE_SOURCE_DIR}/src/PlateRecogService/PlateRecognition
    ${CMAKE_SOURCE_DIR}/src/ControlService
    ${CMAKE_SOURCE_DIR}/src/http_service
    ${CMAKE_SOURCE_DIR}/src/streamserver
    ${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer/src
    ${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer
    ${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer/src/3rdpart
    ${CMAKE_SOURCE_DIR}/src/ViolationDetectionService



    	# ffmpeg
		/usr/local/ffmpeg/include

		# cuda
		/usr/local/cuda/include




)

aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/common SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/app SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/CarDectorService SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/CarDectorService/TrtDetector SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/DrawService SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/TrackingService SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/TrackingService/TrtTracking SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/PlateRecogService SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/PlateRecogService/PlateRecognition SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/ControlService SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/http_service SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/streamserver SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/streamserver/encode SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer/src/net SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer/src/xop SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/streamserver/RtspServer/src/3rdpart/md5 SRC_DIR)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/ViolationDetectionService SRC_DIR)





# include_directories(/home/alice/project/trt_detector/TrtDetector)
# include_directories(/home/alice/project/trt_detector)
# include_directories(/home/alice/project/trt_detector/PlateSegmente)
# include_directories(/home/alice/project/trt_detector/TrtTracking)


# file(GLOB SOURCES "TrtDetector/*.cpp")
# file(GLOB SEG_SOURCES "PlateSegmente/*.cpp")
# file(GLOB TRACK_SOURCES "TrtTracking/*.cpp")
#add_executable(MyExecutable ${SOURCES})

#add_executable(${PROJECT_NAME} /home/alice/project/trt_detector/TrtDetector)
add_executable(${PROJECT_NAME} ${SAMPLES_COMMON_SOURCES} ${SRC_DIR} )

#add OPENCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
#add Tensorrt
include_directories(/home/alice/tensorrt_tar/TensorRT-8.6.1.6/include)
include_directories(/home/alice/tensorrt_tar/TensorRT-8.6.1.6/samples/common)
#include_directories(/home/alice/project/CMakeTest/thirdparty/yolo_plugin/include)
set(TENSORRT_LIB_PATH "/home/alice/tensorrt_tar/TensorRT-8.6.1.6/lib")
file(GLOB LIBS "${TENSORRT_LIB_PATH}/*.so")
# add CUDA
find_package(CUDA 11.6 REQUIRED)
message("CUDA_LIBRARIES:${CUDA_LIBRARIES}")
message("CUDA_INCLUDE_DIRS:${CUDA_INCLUDE_DIRS}")
include_directories(${CUDA_INCLUDE_DIRS})
# link
link_directories(/home/alice/project/trt_detector/thirdparty/yolo_plugin/lib)
# add_executable(${PROJECT_NAME} main.cpp)
#target_link_libraries(pt_unet "${TORCH_LIBRARIES}"  "${OpenCV_LIBS}")
# set_property(TARGET pt_unet PROPERTY CXX_STANDARD 14)

message(${JSONCPP_LIBRARIES})

target_link_libraries(${PROJECT_NAME} hyperlpr3 ${LIBS} ${CUDA_LIBRARIES} ${OpenCV_LIBS} ${TORCH_LIBRARIES} /home/alice/project/trt_detector/thirdparty/yolo_plugin/lib/libyolo_layer.so 		nvinfer
# CallFunIpc

#ffmpeg
avdevice
avformat
avcodec
avfilter
avutil)
#add_executable(app main.cpp)
#add_executable(myapp detector.cpp)
#target_include_directories(myapp PRIVATE ${OpenCV_INCLUDE_DIRS})
#target_link_libraries(myapp ${OpenCV_Lsess = CreateSession(2, 3, 0.3)
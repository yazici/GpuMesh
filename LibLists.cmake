# CUDA
FIND_PACKAGE(CUDA REQUIRED)
LIST(APPEND CUDA_NVCC_FLAGS "-Xptxas;-v;")
LIST(APPEND CUDA_NVCC_FLAGS "-arch=sm_30;")
LIST(APPEND CUDA_NVCC_FLAGS "-std=c++11;--expt-relaxed-constexpr;")
SET(CUDA_PROPAGATE_HOST_FLAGS FALSE)
SET(CUDA_SEPARABLE_COMPILATION ON)


# Qt
FIND_PACKAGE(Qt5OpenGL REQUIRED)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)


# ExTh
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${GpuMesh_SRC_DIR}/../ExperimentalTheatre/")
FIND_PACKAGE(ExperimentalTheatre REQUIRED)


# Global
SET(GpuMesh_LIBRARIES
    ${ExTh_LIBRARIES}
    pthread
    cgns
    optimized "${GpuMesh_SRC_DIR}/../common/apps/Linux_5.4.0/lib/libPir.a"
    debug "${GpuMesh_SRC_DIR}/../common/apps/dbgLinux_5.4.0/lib/libPir.a")
SET(GpuMesh_INCLUDE_DIRS
    ${GpuMesh_SRC_DIR}
    ${ExTh_INCLUDE_DIRS}
    "${GpuMesh_SRC_DIR}/../common/apps/Linux_5.4.0/include")
SET(GpuMesh_QT_MODULES
    OpenGL)

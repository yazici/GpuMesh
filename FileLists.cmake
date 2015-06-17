## Headers ##

# All the header files #
SET(GpuMesh_DATASTRUCTURES_HEADERS
    ${GpuMesh_SRC_DIR}/DataStructures/Mesh.h
    ${GpuMesh_SRC_DIR}/DataStructures/Tetrahedron.h
    ${GpuMesh_SRC_DIR}/DataStructures/TetList.h
    ${GpuMesh_SRC_DIR}/DataStructures/TetPool.h
    ${GpuMesh_SRC_DIR}/DataStructures/Triangle.h
    ${GpuMesh_SRC_DIR}/DataStructures/TriSet.h)


SET(GpuMesh_MESHERS_HEADERS
    ${GpuMesh_SRC_DIR}/Meshers/AbstractMesher.h
    ${GpuMesh_SRC_DIR}/Meshers/CpuMesher.h
    ${GpuMesh_SRC_DIR}/Meshers/GpuMesher.h)

SET(GpuMesh_HEADERS
    ${GpuMesh_DATASTRUCTURES_HEADERS}
    ${GpuMesh_MESHERS_HEADERS}
    ${GpuMesh_SRC_DIR}/GpuMeshCharacter.h)


## Sources ##

# All the source files #
SET(GpuMesh_DATASTRUCTURES_SOURCES
    ${GpuMesh_SRC_DIR}/DataStructures/Mesh.cpp
    ${GpuMesh_SRC_DIR}/DataStructures/TetList.cpp
    ${GpuMesh_SRC_DIR}/DataStructures/TetPool.cpp
    ${GpuMesh_SRC_DIR}/DataStructures/TriSet.cpp)

SET(GpuMesh_MESHERS_SOURCES
    ${GpuMesh_SRC_DIR}/Meshers/AbstractMesher.cpp
    ${GpuMesh_SRC_DIR}/Meshers/CpuMesher.cpp
    ${GpuMesh_SRC_DIR}/Meshers/GpuMesher.cpp)

SET(GpuMesh_SOURCES
    ${GpuMesh_DATASTRUCTURES_SOURCES}
    ${GpuMesh_MESHERS_SOURCES}
    ${GpuMesh_SRC_DIR}/GpuMeshCharacter.cpp
    ${GpuMesh_SRC_DIR}/main.cpp)


## Resrouces ##

# Shaders
SET(GpuMesh_SHADERS
    ${GpuMesh_SRC_DIR}/resources/shaders/LitMesh.vert
    ${GpuMesh_SRC_DIR}/resources/shaders/LitMesh.frag
    ${GpuMesh_SRC_DIR}/resources/shaders/UnlitMesh.vert
    ${GpuMesh_SRC_DIR}/resources/shaders/UnlitMesh.frag
    ${GpuMesh_SRC_DIR}/resources/shaders/Shadow.vert
    ${GpuMesh_SRC_DIR}/resources/shaders/Shadow.frag
    ${GpuMesh_SRC_DIR}/resources/shaders/Backdrop.vert
    ${GpuMesh_SRC_DIR}/resources/shaders/Backdrop.frag
    ${GpuMesh_SRC_DIR}/resources/shaders/BloomBlur.vert
    ${GpuMesh_SRC_DIR}/resources/shaders/BloomBlur.frag
    ${GpuMesh_SRC_DIR}/resources/shaders/BloomBlend.vert
    ${GpuMesh_SRC_DIR}/resources/shaders/BloomBlend.frag)

# Qrc File
QT5_ADD_RESOURCES(GpuMesh_RESOURCES
    ${GpuMesh_SRC_DIR}/resources/GpuMesh.qrc)


## Global ##
SET(GpuMesh_CONFIG_FILES
    ${GpuMesh_SRC_DIR}/CMakeLists.txt
    ${GpuMesh_SRC_DIR}/FileLists.cmake
    ${GpuMesh_SRC_DIR}/LibLists.cmake)
	
SET(GpuMesh_SRC_FILES
    ${GpuMesh_HEADERS}
    ${GpuMesh_SOURCES}
    ${GpuMesh_SHADERS}
    ${GpuMesh_RESOURCES}
    ${GpuMesh_CONFIG_FILES}
    ${GpuMesh_MOC_CPP_FILES})

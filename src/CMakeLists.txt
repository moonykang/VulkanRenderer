SET(SUBDIRECTORIES
    application
    launcher
    model
    platform
    render
    rhi
    scene
    vulkan
)

FOREACH(DIRECTORY ${SUBDIRECTORIES})
    ADD_SUBDIRECTORY(${DIRECTORY})
ENDFOREACH()

SET(SOURCES
   ${SOURCES}
)
SET(HEADERS
   ${HEADERS}
)

SET(LAUNCHER_SOURCES ${LAUNCHER_SOURCES})
SET(LAUNCHER_HEADERS ${LAUNCHER_HEADERS})

SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "sources" FILES ${SOURCES} ${HEADERS})

IF(WIN32) # For window
    ADD_LIBRARY(
        renderer
        SHARED
        ${SOURCES} ${HEADERS}
    )

    ADD_DEPENDENCIES(renderer Assets)

    TARGET_LINK_LIBRARIES(
        renderer
        libktx
        ${VULKAN_LIBRARY}
    )

    ADD_EXECUTABLE(
        launcher
        WIN32
        ${LAUNCHER_SOURCES} ${LAUNCHER_HEADERS}
    )

    TARGET_LINK_LIBRARIES(
        launcher
        renderer
        libktx
        ${VULKAN_LIBRARY}
    )
ELSE() # For android
    ADD_LIBRARY(
        renderer
        SHARED
        ${SOURCES} ${HEADERS} ${LAUNCHER_SOURCES} ${LAUNCHER_HEADERS}
    )

    ADD_DEPENDENCIES(renderer Assets)

    TARGET_LINK_LIBRARIES(
        renderer
        libktx
        ${VULKAN_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${ANDROID_LIBRARIES}
    )
ENDIF()
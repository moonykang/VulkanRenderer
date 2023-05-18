FILE(GLOB_RECURSE shader_files shaders/*.vert shaders/*.frag shaders/*.comp shaders/*.rgen shaders/*.rchit shaders/*.rint shaders/*.rmiss)

FIND_PROGRAM(GLSLANG_VALIDATOR
	NAMES glslangValidator
	HINTS ENV VULKAN_SDK
	PATH_SUFFIXES bin)

FIND_PROGRAM(GLSLC
    glslc
    HINTS ${ANDROID_NDK}/shader-tools/*/ ${CMAKE_ANDROID_NDK}/shader-tools/*/)

IF(GLSLC)
	SET(SHADER_COMPILER ${GLSLC} --target-spv=spv1.4)
ENDIF()

IF(GLSLANG_VALIDATOR)
	SET(SHADER_COMPILER ${GLSLANG_VALIDATOR} --target-env vulkan1.2 -V)
ENDIF()

IF(NOT GLSLANG_VALIDATOR AND NOT GLSLC)
	MESSAGE(FATAL_ERROR "Unable to find glslangValidator or glslc shader compiler")
ENDIF()

FOREACH(shader ${shader_files})
	GET_FILENAME_COMPONENT(file_name ${shader} NAME)
    GET_FILENAME_COMPONENT(file_last_ext ${shader} LAST_EXT)
	GET_FILENAME_COMPONENT(full_path ${shader} ABSOLUTE)
    GET_FILENAME_COMPONENT(directory ${shader} DIRECTORY)
    STRING(REPLACE "." "" file_ext ${file_last_ext})
	SET(output_file ${directory}/${file_name}.spv)

	SET(compiled_shaders ${compiled_shaders} ${output_file})
	SET(compiled_shaders ${compiled_shaders} PARENT_SCOPE)

	MESSAGE("${SHADER_COMPILER} ${full_path} -o ${output_file}")
	ADD_CUSTOM_COMMAND(
		OUTPUT ${output_file}
		COMMAND ${SHADER_COMPILER} ${full_path} -o ${output_file}
		DEPENDS ${full_path}
	)
ENDFOREACH()

FILE(GLOB_RECURSE shader_extra_files ${shader_extra_files} shaders/*.glsl)

SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${shader_files} ${shader_extra_files})
ADD_CUSTOM_TARGET(
	Assets 
	DEPENDS ${compiled_shaders}
	SOURCES ${shader_files} ${shader_extra_files})

# Development Tools

# Check if tools directory has source files
file(GLOB TOOL_SOURCES "*.c")

if(TOOL_SOURCES)
    # Common tools library if there are common files
    file(GLOB COMMON_SOURCES "common/*.c")
    if(COMMON_SOURCES)
        add_library(asthra_tools_common STATIC ${COMMON_SOURCES})
        
        target_include_directories(asthra_tools_common
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/common
        )
        
        target_link_libraries(asthra_tools_common
            PUBLIC
                asthra_compiler
        )
    endif()
    
    # Individual tools
    foreach(tool_source ${TOOL_SOURCES})
        get_filename_component(tool_name ${tool_source} NAME_WE)
        
        add_executable(${tool_name} ${tool_source})
        
        if(TARGET asthra_tools_common)
            target_link_libraries(${tool_name} 
                asthra_tools_common
                asthra_compiler
            )
        else()
            target_link_libraries(${tool_name} asthra_compiler)
        endif()
        
        install(TARGETS ${tool_name}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endforeach()
endif()

# Check for subdirectories with tools
file(GLOB TOOL_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")
foreach(subdir ${TOOL_SUBDIRS})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}" AND NOT subdir STREQUAL "common")
        file(GLOB SUBDIR_SOURCES "${subdir}/*.c")
        if(SUBDIR_SOURCES)
            # Create tool from subdirectory
            get_filename_component(tool_name ${subdir} NAME)
            
            add_executable(${tool_name} ${SUBDIR_SOURCES})
            target_link_libraries(${tool_name} asthra_compiler)
            
            if(TARGET asthra_tools_common)
                target_link_libraries(${tool_name} asthra_tools_common)
            endif()
            
            install(TARGETS ${tool_name}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        endif()
    endif()
endforeach() 
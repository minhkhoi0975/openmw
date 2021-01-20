function (get_whole_archive_options OUT_VAR)
    # We use --whole-archive because OSG plugins use registration.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(${OUT_VAR} -Wl,--whole-archive ${ARGN} -Wl,--no-whole-archive PARENT_SCOPE)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(${OUT_VAR} -Wl,-all_load ${ARGN} -Wl,-noall_load PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "get_whole_archive_options not implemented for CMAKE_CXX_COMPILER_ID ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction (get_whole_archive_options)

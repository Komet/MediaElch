# cmake-format: off
#mkdir build/coverage
#lcov -d build/source/tests/fiblib-test/CMakeFiles/fiblib-test.dir -c -o build/coverage/fiblib-test.info
#genhtml -o build/coverage/html build/coverage/fiblib-test.info

set(LCOV_EXCLUDE_COVERAGE)

# Function to register the target for coverage.
function(generate_lcov_report coverage_target target)
    if(NOT TARGET coverage-init)
        # Create initialize coverage target. Used for zeroing out counters, etc
        add_custom_target(
            coverage-zero
            COMMAND
                ${lcov_EXECUTABLE}
                    --zerocounters
                    --base-directory ${CMAKE_BINARY_DIR}
                    --directory ${CMAKE_SOURCE_DIR}
                    -q
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_target(
            coverage-init
            COMMAND
                ${lcov_EXECUTABLE}
                    --no-external
                    --capture
                    --initial
                    --base-directory ${CMAKE_BINARY_DIR}
                    --directory ${CMAKE_SOURCE_DIR}
                    --output-file ${CMAKE_BINARY_DIR}/coverage-base.info
                    -q
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            BYPRODUCTS ${CMAKE_BINARY_DIR}/coverage-base.info
        )

        add_custom_target(
            coverage-info
            COMMAND
                ${lcov_EXECUTABLE}
                    --capture
                    --no-external
                    --base-directory ${CMAKE_BINARY_DIR}
                    --directory ${CMAKE_SOURCE_DIR}
                    --output-file ${CMAKE_BINARY_DIR}/coverage-captured.info
                    -q
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            BYPRODUCTS ${CMAKE_BINARY_DIR}/coverage-captured.info
        )

        add_custom_target(
            coverage-merge
            COMMAND
                ${lcov_EXECUTABLE}
                    --add-tracefile ${CMAKE_BINARY_DIR}/coverage-base.info
                    --add-tracefile ${CMAKE_BINARY_DIR}/coverage-captured.info
                    --output-file ${CMAKE_BINARY_DIR}/coverage-merged.info
                    -q
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            BYPRODUCTS ${CMAKE_BINARY_DIR}/coverage-merged.info
        )

        add_custom_target(
            coverage-filter
            COMMAND
                ${lcov_EXECUTABLE}
                    --base-directory ${CMAKE_BINARY_DIR}
                    --directory ${CMAKE_SOURCE_DIR}
                    --remove ${CMAKE_BINARY_DIR}/coverage-merged.info
                    ${LCOV_EXCLUDE_COVERAGE}
                    --output-file ${CMAKE_BINARY_DIR}/coverage-filtered.info
                    -q
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            BYPRODUCTS ${CMAKE_BINARY_DIR}/coverage-filtered.info
        )

        add_custom_target(
            coverage-report
            COMMAND
                ${genhtml_EXECUTABLE}
                    --output-directory ${CMAKE_BINARY_DIR}/coverage
                    --title "${PROJECT_NAME} Test Coverage"
                    --num-spaces 4
                    ${CMAKE_BINARY_DIR}/coverage-filtered.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${CMAKE_BINARY_DIR}/coverage-filtered.info
        )

        add_dependencies(coverage-init coverage-zero)
        add_dependencies(coverage-info coverage-init)
        add_dependencies(coverage-merge coverage-info)
        add_dependencies(coverage-filter coverage-merge)
        add_dependencies(coverage-report coverage-filter)
        add_dependencies(coverage coverage-report)
    endif()

    add_custom_target(${coverage_target}
        COMMAND $<TARGET_FILE:${target}>
    )

    add_dependencies(coverage-info ${coverage_target})
    add_dependencies(${coverage_target} coverage-init)

endfunction()
# cmake-format: on

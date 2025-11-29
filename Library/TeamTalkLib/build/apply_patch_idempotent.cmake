if(NOT PATCH_FILE)
  message(FATAL_ERROR "PATCH_FILE must be specified")
endif()

if(NOT SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR must be specified")
endif()

if(NOT EXISTS "${PATCH_FILE}")
  message(FATAL_ERROR "Patch file does not exist: ${PATCH_FILE}")
endif()

if(NOT EXISTS "${SOURCE_DIR}")
  message(FATAL_ERROR "Source directory does not exist: ${SOURCE_DIR}")
endif()

execute_process(
  COMMAND git apply --reverse --check ${PATCH_FILE}
  WORKING_DIRECTORY ${SOURCE_DIR}
  RESULT_VARIABLE REVERSE_CHECK_RESULT
  OUTPUT_QUIET
  ERROR_QUIET
)

if(REVERSE_CHECK_RESULT EQUAL 0)
  message(STATUS "Patch already applied (skipping): ${PATCH_FILE}")
else()
  execute_process(
    COMMAND git apply --check ${PATCH_FILE}
    WORKING_DIRECTORY ${SOURCE_DIR}
    RESULT_VARIABLE APPLY_CHECK_RESULT
    OUTPUT_VARIABLE CHECK_OUTPUT
    ERROR_VARIABLE CHECK_ERROR
  )

  if(APPLY_CHECK_RESULT EQUAL 0)
    execute_process(
      COMMAND git apply ${PATCH_FILE}
      WORKING_DIRECTORY ${SOURCE_DIR}
      RESULT_VARIABLE APPLY_RESULT
      OUTPUT_VARIABLE APPLY_OUTPUT
      ERROR_VARIABLE APPLY_ERROR
    )

    if(APPLY_RESULT EQUAL 0)
      message(STATUS "Successfully applied patch: ${PATCH_FILE}")
    else()
      message(FATAL_ERROR "Failed to apply patch: ${PATCH_FILE}\nError: ${APPLY_ERROR}")
    endif()
  else()
    message(FATAL_ERROR "Patch cannot be applied (conflicts or corruption): ${PATCH_FILE}\nError: ${CHECK_ERROR}")
  endif()
endif()

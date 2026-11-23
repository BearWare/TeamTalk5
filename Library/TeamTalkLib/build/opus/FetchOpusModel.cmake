if(NOT OPUS_SRC_DIR)
  message(FATAL_ERROR "OPUS_SRC_DIR must be provided")
endif()

set(_autogen "${OPUS_SRC_DIR}/autogen.sh")
if(NOT EXISTS "${_autogen}")
  message(FATAL_ERROR "autogen.sh not found at ${_autogen}")
endif()

file(READ "${_autogen}" _autogen_content)
string(REGEX MATCH "dnn/download_model\\.sh[ \t]+\"([a-f0-9]+)\"" _match "${_autogen_content}")
set(OPUS_MODEL_SHA "${CMAKE_MATCH_1}")
if(NOT OPUS_MODEL_SHA)
  message(FATAL_ERROR "Could not parse opus model SHA from ${_autogen}")
endif()

set(_tgz_name "opus_data-${OPUS_MODEL_SHA}.tar.gz")
set(_tgz_path "${OPUS_SRC_DIR}/${_tgz_name}")
set(_url "https://media.xiph.org/opus/models/${_tgz_name}")

if(NOT EXISTS "${_tgz_path}")
  message(STATUS "Downloading Opus DNN model: ${_tgz_name}")
  file(DOWNLOAD "${_url}" "${_tgz_path}"
       EXPECTED_HASH SHA256=${OPUS_MODEL_SHA}
       TLS_VERIFY ON
       SHOW_PROGRESS
       STATUS _dl_status)
  list(GET _dl_status 0 _dl_code)
  if(NOT _dl_code EQUAL 0)
    file(REMOVE "${_tgz_path}")
    list(GET _dl_status 1 _dl_msg)
    message(FATAL_ERROR "Download of ${_url} failed: ${_dl_msg}")
  endif()
endif()

set(_marker "${OPUS_SRC_DIR}/.opus_model_extracted_${OPUS_MODEL_SHA}")
if(NOT EXISTS "${_marker}")
  message(STATUS "Extracting ${_tgz_name} into ${OPUS_SRC_DIR}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xzf "${_tgz_path}"
    WORKING_DIRECTORY "${OPUS_SRC_DIR}"
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "Failed to extract ${_tgz_path} (exit ${_rc})")
  endif()
  file(WRITE "${_marker}" "${OPUS_MODEL_SHA}\n")
endif()

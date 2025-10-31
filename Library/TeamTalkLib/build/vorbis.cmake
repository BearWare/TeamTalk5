if (TOOLCHAIN_VORBIS)

  set (VORBIS_LINK_FLAGS vorbisfile vorbisenc vorbis ogg)

else()

  # Ubuntu: libvorbis-dev
  find_library(VORBIS_LIBRARY vorbis)
  find_library(VORBISENC_LIBRARY vorbisenc)
  find_library(VORBISFILE_LIBRARY vorbisfile)
  set (VORBIS_LINK_FLAGS ${VORBIS_LIBRARY} ${VORBISENC_LIBRARY} ${VORBISFILE_LIBRARY})

endif()

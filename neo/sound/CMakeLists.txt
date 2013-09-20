SET( SOUND_C 
	OggVorbis/oggsrc/bitwise.c
	OggVorbis/oggsrc/framing.c
	OggVorbis/vorbissrc/analysis.c
	OggVorbis/vorbissrc/barkmel.c
	OggVorbis/vorbissrc/bitrate.c
	OggVorbis/vorbissrc/block.c
	OggVorbis/vorbissrc/codebook.c
	OggVorbis/vorbissrc/envelope.c
	OggVorbis/vorbissrc/floor0.c
	OggVorbis/vorbissrc/floor1.c
	OggVorbis/vorbissrc/info.c
	OggVorbis/vorbissrc/lookup.c
	OggVorbis/vorbissrc/lpc.c
	OggVorbis/vorbissrc/lsp.c
	OggVorbis/vorbissrc/mapping0.c
	OggVorbis/vorbissrc/mdct.c
	OggVorbis/vorbissrc/psy.c
	OggVorbis/vorbissrc/registry.c
	OggVorbis/vorbissrc/res0.c
	OggVorbis/vorbissrc/sharedbook.c
	OggVorbis/vorbissrc/smallft.c
	OggVorbis/vorbissrc/synthesis.c
	OggVorbis/vorbissrc/vorbisenc.c
	OggVorbis/vorbissrc/vorbisfile.c
	OggVorbis/vorbissrc/windowvb.c
)

SET( SOURCES
	${SOUND_C}
)

INCLUDE_DIRECTORIES(
	OggVorbis
	OggVorbis/vorbissrc
)

add_library( sound ${SOURCES} )
set_target_properties( sound PROPERTIES PREFIX "")

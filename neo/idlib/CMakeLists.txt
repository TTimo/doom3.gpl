FILE( GLOB IDLIB_CPP *.cpp )
FILE( GLOB IDLIB_HDR *.h )
FILE( GLOB BV_CPP bv/*.cpp )
FILE( GLOB CONT_CPP containers/*.cpp )
FILE( GLOB HASH_CPP hashing/*.cpp )
FILE( GLOB MATH_CPP math/*.cpp )
FILE( GLOB GEOM_CPP geometry/*.cpp )

SET( SOURCES
	${IDLIB_CPP}
	${IDLIB_HDR}
	${BV_CPP}
	${CONT_CPP}
	${HASH_CPP}
	${MATH_CPP}
	${GEOM_CPP}
)

INCLUDE_DIRECTORIES()

ADD_DEFINITIONS( ${CPP_WARNING_OPTIONS} )
add_library( idlib ${SOURCES} )
set_target_properties( idlib PROPERTIES PREFIX "")


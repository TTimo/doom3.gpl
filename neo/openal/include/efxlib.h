/*
*/

#ifndef __EFXLIBH
#define __EFXLIBH

#include "eax4.h"




///////////////////////////////////////////////////////////
// Class definitions.
class idSoundEffect
{
public:
	idSoundEffect() {
	};
	~idSoundEffect() { 
		if ( data && datasize ) {
			Mem_Free( data );
			data = NULL;
		}
	}
	
	idStr name;
	int datasize;
	void *data;
};

class idEFXFile
{
private:

protected:
    // Protected data members.

public:
    // Public data members.

private:
    
public:
	idEFXFile();
	~idEFXFile();

	bool FindEffect( idStr &name, idSoundEffect **effect, int *index );
	bool ReadEffect( idLexer &lexer, idSoundEffect *effect );
	bool LoadFile( const char *filename, bool OSPath = false );
	void UnloadFile( void );
	void Clear( void );

	idList<idSoundEffect *>effects;
};
///////////////////////////////////////////////////////////




#endif // __EFXLIBH


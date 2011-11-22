/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

/*
===============================================================================

	idCompressor is a layer ontop of idFile which provides lossless data
	compression. The compressor can be used as a regular file and multiple
	compressors can be stacked ontop of each other.

===============================================================================
*/

class idCompressor : public idFile {
public:
							// compressor allocation
	static idCompressor *	AllocNoCompression( void );
	static idCompressor *	AllocBitStream( void );
	static idCompressor *	AllocRunLength( void );
	static idCompressor *	AllocRunLength_ZeroBased( void );
	static idCompressor *	AllocHuffman( void );
	static idCompressor *	AllocArithmetic( void );
	static idCompressor *	AllocLZSS( void );
	static idCompressor *	AllocLZSS_WordAligned( void );
	static idCompressor *	AllocLZW( void );

							// initialization
	virtual void			Init( idFile *f, bool compress, int wordLength ) = 0;
	virtual void			FinishCompress( void ) = 0;
	virtual float			GetCompressionRatio( void ) const = 0;

							// common idFile interface
	virtual const char *	GetName( void ) = 0;
	virtual const char *	GetFullPath( void ) = 0;
	virtual int				Read( void *outData, int outLength ) = 0;
	virtual int				Write( const void *inData, int inLength ) = 0;
	virtual int				Length( void ) = 0;
	virtual ID_TIME_T			Timestamp( void ) = 0;
	virtual int				Tell( void ) = 0;
	virtual void			ForceFlush( void ) = 0;
	virtual void			Flush( void ) = 0;
	virtual int				Seek( long offset, fsOrigin_t origin ) = 0;
};

#endif /* !__COMPRESSOR_H__ */

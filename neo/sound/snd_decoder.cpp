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

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "snd_local.h"
#include "OggVorbis/vorbis/codec.h"
#include "OggVorbis/vorbis/vorbisfile.h"


/*
===================================================================================

  Thread safe decoder memory allocator.

  Each OggVorbis decoder consumes about 150kB of memory.

===================================================================================
*/

idDynamicBlockAlloc<byte, 1<<20, 128>		decoderMemoryAllocator;

const int MIN_OGGVORBIS_MEMORY				= 768 * 1024;

extern "C" {
	void *_decoder_malloc( size_t size );
	void *_decoder_calloc( size_t num, size_t size );
	void *_decoder_realloc( void *memblock, size_t size );
	void _decoder_free( void *memblock );
}

void *_decoder_malloc( size_t size ) {
	void *ptr = decoderMemoryAllocator.Alloc( size );
	assert( size == 0 || ptr != NULL );
	return ptr;
}

void *_decoder_calloc( size_t num, size_t size ) {
	void *ptr = decoderMemoryAllocator.Alloc( num * size );
	assert( ( num * size ) == 0 || ptr != NULL );
	memset( ptr, 0, num * size );
	return ptr;
}

void *_decoder_realloc( void *memblock, size_t size ) {
	void *ptr = decoderMemoryAllocator.Resize( (byte *)memblock, size );
	assert( size == 0 || ptr != NULL );
	return ptr;
}

void _decoder_free( void *memblock ) {
	decoderMemoryAllocator.Free( (byte *)memblock );
}


/*
===================================================================================

  OggVorbis file loading/decoding.

===================================================================================
*/

/*
====================
FS_ReadOGG
====================
*/
size_t FS_ReadOGG( void *dest, size_t size1, size_t size2, void *fh ) {
	idFile *f = reinterpret_cast<idFile *>(fh);
	return f->Read( dest, size1 * size2 );
}

/*
====================
FS_SeekOGG
====================
*/
int FS_SeekOGG( void *fh, ogg_int64_t to, int type ) {
	fsOrigin_t retype = FS_SEEK_SET;

	if ( type == SEEK_CUR ) {
		retype = FS_SEEK_CUR;
	} else if ( type == SEEK_END ) {
		retype = FS_SEEK_END;
	} else if ( type == SEEK_SET ) {
		retype = FS_SEEK_SET;
	} else {
		common->FatalError( "fs_seekOGG: seek without type\n" );
	}
	idFile *f = reinterpret_cast<idFile *>(fh);
	return f->Seek( to, retype );
}

/*
====================
FS_CloseOGG
====================
*/
int FS_CloseOGG( void *fh ) {
	return 0;
}

/*
====================
FS_TellOGG
====================
*/
long FS_TellOGG( void *fh ) {
	idFile *f = reinterpret_cast<idFile *>(fh);
	return f->Tell();
}

/*
====================
ov_openFile
====================
*/
int ov_openFile( idFile *f, OggVorbis_File *vf ) {
	ov_callbacks callbacks;

	memset( vf, 0, sizeof( OggVorbis_File ) );

	callbacks.read_func = FS_ReadOGG;
	callbacks.seek_func = FS_SeekOGG;
	callbacks.close_func = FS_CloseOGG;
	callbacks.tell_func = FS_TellOGG;
	return ov_open_callbacks((void *)f, vf, NULL, -1, callbacks);
}

/*
====================
idWaveFile::OpenOGG
====================
*/
int idWaveFile::OpenOGG( const char* strFileName, waveformatex_t *pwfx ) {
	OggVorbis_File *ov;

	memset( pwfx, 0, sizeof( waveformatex_t ) );

	mhmmio = fileSystem->OpenFileRead( strFileName );
	if ( !mhmmio ) {
		return -1;
	}

	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );

	ov = new OggVorbis_File;

	if( ov_openFile( mhmmio, ov ) < 0 ) {
		delete ov;
		Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
		return -1;
	}

	mfileTime = mhmmio->Timestamp();

	vorbis_info *vi = ov_info( ov, -1 );

	mpwfx.Format.nSamplesPerSec = vi->rate;
	mpwfx.Format.nChannels = vi->channels;
	mpwfx.Format.wBitsPerSample = sizeof(short) * 8;
	mdwSize = ov_pcm_total( ov, -1 ) * vi->channels;	// pcm samples * num channels
	mbIsReadingFromMemory = false;

	if ( idSoundSystemLocal::s_realTimeDecoding.GetBool() ) {

		ov_clear( ov );
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
		delete ov;

		mpwfx.Format.wFormatTag = WAVE_FORMAT_TAG_OGG;
		mhmmio = fileSystem->OpenFileRead( strFileName );
		mMemSize = mhmmio->Length();

	} else {

		ogg = ov;

		mpwfx.Format.wFormatTag = WAVE_FORMAT_TAG_PCM;
		mMemSize = mdwSize * sizeof( short );
	}

	memcpy( pwfx, &mpwfx, sizeof( waveformatex_t ) );

	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );

	isOgg = true;

	return 0;
}

/*
====================
idWaveFile::ReadOGG
====================
*/
int idWaveFile::ReadOGG( byte* pBuffer, int dwSizeToRead, int *pdwSizeRead ) {
	int total = dwSizeToRead;
	char *bufferPtr = (char *)pBuffer;
	OggVorbis_File *ov = (OggVorbis_File *) ogg;

	do {
		int ret = ov_read( ov, bufferPtr, total >= 4096 ? 4096 : total, Swap_IsBigEndian(), 2, 1, &ov->stream );
		if ( ret == 0 ) {
			break;
		}
		if ( ret < 0 ) {
			return -1;
		}
		bufferPtr += ret;
		total -= ret;
	} while( total > 0 );

	dwSizeToRead = (byte *)bufferPtr - pBuffer;

	if ( pdwSizeRead != NULL ) {
		*pdwSizeRead = dwSizeToRead;
	}

	return dwSizeToRead;
}

/*
====================
idWaveFile::CloseOGG
====================
*/
int idWaveFile::CloseOGG( void ) {
	OggVorbis_File *ov = (OggVorbis_File *) ogg;
	if ( ov != NULL ) {
		Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );
		ov_clear( ov );
		delete ov;
		Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
		ogg = NULL;
		return 0;
	}
	return -1;
}


/*
===================================================================================

  idSampleDecoderLocal

===================================================================================
*/

class idSampleDecoderLocal : public idSampleDecoder {
public:
	virtual void			Decode( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest );
	virtual void			ClearDecoder( void );
	virtual idSoundSample *	GetSample( void ) const;
	virtual int				GetLastDecodeTime( void ) const;

	void					Clear( void );
	int						DecodePCM( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest );
	int						DecodeOGG( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest );

private:
	bool					failed;				// set if decoding failed
	int						lastFormat;			// last format being decoded
	idSoundSample *			lastSample;			// last sample being decoded
	int						lastSampleOffset;	// last offset into the decoded sample
	int						lastDecodeTime;		// last time decoding sound
	idFile_Memory			file;				// encoded file in memory

	OggVorbis_File			ogg;				// OggVorbis file
};

idBlockAlloc<idSampleDecoderLocal, 64>		sampleDecoderAllocator;

/*
====================
idSampleDecoder::Init
====================
*/
void idSampleDecoder::Init( void ) {
	decoderMemoryAllocator.Init();
	decoderMemoryAllocator.SetLockMemory( true );
	decoderMemoryAllocator.SetFixedBlocks( idSoundSystemLocal::s_realTimeDecoding.GetBool() ? 10 : 1 );
}

/*
====================
idSampleDecoder::Shutdown
====================
*/
void idSampleDecoder::Shutdown( void ) {
	decoderMemoryAllocator.Shutdown();
	sampleDecoderAllocator.Shutdown();
}

/*
====================
idSampleDecoder::Alloc
====================
*/
idSampleDecoder *idSampleDecoder::Alloc( void ) {
	idSampleDecoderLocal *decoder = sampleDecoderAllocator.Alloc();
	decoder->Clear();
	return decoder;
}

/*
====================
idSampleDecoder::Free
====================
*/
void idSampleDecoder::Free( idSampleDecoder *decoder ) {
	idSampleDecoderLocal *localDecoder = static_cast<idSampleDecoderLocal *>( decoder );
	localDecoder->ClearDecoder();
	sampleDecoderAllocator.Free( localDecoder );
}

/*
====================
idSampleDecoder::GetNumUsedBlocks
====================
*/
int idSampleDecoder::GetNumUsedBlocks( void ) {
	return decoderMemoryAllocator.GetNumUsedBlocks();
}

/*
====================
idSampleDecoder::GetUsedBlockMemory
====================
*/
int idSampleDecoder::GetUsedBlockMemory( void ) {
	return decoderMemoryAllocator.GetUsedBlockMemory();
}

/*
====================
idSampleDecoderLocal::Clear
====================
*/
void idSampleDecoderLocal::Clear( void ) {
	failed = false;
	lastFormat = WAVE_FORMAT_TAG_PCM;
	lastSample = NULL;
	lastSampleOffset = 0;
	lastDecodeTime = 0;
}

/*
====================
idSampleDecoderLocal::ClearDecoder
====================
*/
void idSampleDecoderLocal::ClearDecoder( void ) {
	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );

	switch( lastFormat ) {
		case WAVE_FORMAT_TAG_PCM: {
			break;
		}
		case WAVE_FORMAT_TAG_OGG: {
			ov_clear( &ogg );
			memset( &ogg, 0, sizeof( ogg ) );
			break;
		}
	}

	Clear();

	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
}

/*
====================
idSampleDecoderLocal::GetSample
====================
*/
idSoundSample *idSampleDecoderLocal::GetSample( void ) const {
	return lastSample;
}

/*
====================
idSampleDecoderLocal::GetLastDecodeTime
====================
*/
int idSampleDecoderLocal::GetLastDecodeTime( void ) const {
	return lastDecodeTime;
}

/*
====================
idSampleDecoderLocal::Decode
====================
*/
void idSampleDecoderLocal::Decode( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest ) {
	int readSamples44k;

	if ( sample->objectInfo.wFormatTag != lastFormat || sample != lastSample ) {
		ClearDecoder();
	}

	lastDecodeTime = soundSystemLocal.CurrentSoundTime;

	if ( failed ) {
		memset( dest, 0, sampleCount44k * sizeof( dest[0] ) );
		return;
	}

	// samples can be decoded both from the sound thread and the main thread for shakes
	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );

	switch( sample->objectInfo.wFormatTag ) {
		case WAVE_FORMAT_TAG_PCM: {
			readSamples44k = DecodePCM( sample, sampleOffset44k, sampleCount44k, dest );
			break;
		}
		case WAVE_FORMAT_TAG_OGG: {
			readSamples44k = DecodeOGG( sample, sampleOffset44k, sampleCount44k, dest );
			break;
		}
		default: {
			readSamples44k = 0;
			break;
		}
	}

	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );

	if ( readSamples44k < sampleCount44k ) {
		memset( dest + readSamples44k, 0, ( sampleCount44k - readSamples44k ) * sizeof( dest[0] ) );
	}
}

/*
====================
idSampleDecoderLocal::DecodePCM
====================
*/
int idSampleDecoderLocal::DecodePCM( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest ) {
	const byte *first;
	int pos, size, readSamples;

	lastFormat = WAVE_FORMAT_TAG_PCM;
	lastSample = sample;

	int shift = 22050 / sample->objectInfo.nSamplesPerSec;
	int sampleOffset = sampleOffset44k >> shift;
	int sampleCount = sampleCount44k >> shift;

	if ( sample->nonCacheData == NULL ) {
		assert( false );	// this should never happen ( note: I've seen that happen with the main thread down in idGameLocal::MapClear clearing entities - TTimo )
		failed = true;
		return 0;
	}

	if ( !sample->FetchFromCache( sampleOffset * sizeof( short ), &first, &pos, &size, false ) ) {
		failed = true;
		return 0;
	}

	if ( size - pos < sampleCount * sizeof( short ) ) {
		readSamples = ( size - pos ) / sizeof( short );
	} else {
		readSamples = sampleCount;
	}

	// duplicate samples for 44kHz output
	SIMDProcessor->UpSamplePCMTo44kHz( dest, (const short *)(first+pos), readSamples, sample->objectInfo.nSamplesPerSec, sample->objectInfo.nChannels );

	return ( readSamples << shift );
}

/*
====================
idSampleDecoderLocal::DecodeOGG
====================
*/
int idSampleDecoderLocal::DecodeOGG( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest ) {
	int readSamples, totalSamples;

	int shift = 22050 / sample->objectInfo.nSamplesPerSec;
	int sampleOffset = sampleOffset44k >> shift;
	int sampleCount = sampleCount44k >> shift;

	// open OGG file if not yet opened
	if ( lastSample == NULL ) {
		// make sure there is enough space for another decoder
		if ( decoderMemoryAllocator.GetFreeBlockMemory() < MIN_OGGVORBIS_MEMORY ) {
			return 0;
		}
		if ( sample->nonCacheData == NULL ) {
			assert( false );	// this should never happen
			failed = true;
			return 0;
		}
		file.SetData( (const char *)sample->nonCacheData, sample->objectMemSize );
		if ( ov_openFile( &file, &ogg ) < 0 ) {
			failed = true;
			return 0;
		}
		lastFormat = WAVE_FORMAT_TAG_OGG;
		lastSample = sample;
	}

	// seek to the right offset if necessary
	if ( sampleOffset != lastSampleOffset ) {
		if ( ov_pcm_seek( &ogg, sampleOffset / sample->objectInfo.nChannels ) != 0 ) {
			failed = true;
			return 0;
		}
	}

	lastSampleOffset = sampleOffset;

	// decode OGG samples
	totalSamples = sampleCount;
	readSamples = 0;
	do {
		float **samples;
		int ret = ov_read_float( &ogg, &samples, totalSamples / sample->objectInfo.nChannels, &ogg.stream );
		if ( ret == 0 ) {
			failed = true;
			break;
		}
		if ( ret < 0 ) {
			failed = true;
			return 0;
		}
		ret *= sample->objectInfo.nChannels;

		SIMDProcessor->UpSampleOGGTo44kHz( dest + ( readSamples << shift ), samples, ret, sample->objectInfo.nSamplesPerSec, sample->objectInfo.nChannels );

		readSamples += ret;
		totalSamples -= ret;
	} while( totalSamples > 0 );

	lastSampleOffset += readSamples;

	return ( readSamples << shift );
}

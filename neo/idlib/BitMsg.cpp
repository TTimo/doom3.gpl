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

#include "precompiled.h"
#pragma hdrstop


/*
==============================================================================

  idBitMsg

==============================================================================
*/

/*
================
idBitMsg::idBitMsg
================
*/
idBitMsg::idBitMsg() {
	writeData = NULL;
	readData = NULL;
	maxSize = 0;
	curSize = 0;
	writeBit = 0;
	readCount = 0;
	readBit = 0;
	allowOverflow = false;
	overflowed = false;
}

/*
================
idBitMsg::CheckOverflow
================
*/
bool idBitMsg::CheckOverflow( int numBits ) {
	assert( numBits >= 0 );
	if ( numBits > GetRemainingWriteBits() ) {
		if ( !allowOverflow ) {
			idLib::common->FatalError( "idBitMsg: overflow without allowOverflow set" );
		}
		if ( numBits > ( maxSize << 3 ) ) {
			idLib::common->FatalError( "idBitMsg: %i bits is > full message size", numBits );
		}
		idLib::common->Printf( "idBitMsg: overflow\n" );
		BeginWriting();
		overflowed = true;
		return true;
	}
	return false;
}

/*
================
idBitMsg::GetByteSpace
================
*/
byte *idBitMsg::GetByteSpace( int length ) {
	byte *ptr;

	if ( !writeData ) {
		idLib::common->FatalError( "idBitMsg::GetByteSpace: cannot write to message" );
	}

	// round up to the next byte
	WriteByteAlign();

	// check for overflow
	CheckOverflow( length << 3 );

	ptr = writeData + curSize;
	curSize += length;
	return ptr;
}

/*
================
idBitMsg::WriteBits

  If the number of bits is negative a sign is included.
================
*/
void idBitMsg::WriteBits( int value, int numBits ) {
	int		put;
	int		fraction;

	if ( !writeData ) {
		idLib::common->Error( "idBitMsg::WriteBits: cannot write to message" );
	}

	// check if the number of bits is valid
	if ( numBits == 0 || numBits < -31 || numBits > 32 ) {
		idLib::common->Error( "idBitMsg::WriteBits: bad numBits %i", numBits );
	}

	// check for value overflows
	// this should be an error really, as it can go unnoticed and cause either bandwidth or corrupted data transmitted
	if ( numBits != 32 ) {
		if ( numBits > 0 ) {
			if ( value > ( 1 << numBits ) - 1 ) {
				idLib::common->Warning( "idBitMsg::WriteBits: value overflow %d %d", value, numBits );
			} else if ( value < 0 ) {
				idLib::common->Warning( "idBitMsg::WriteBits: value overflow %d %d", value, numBits );
			}
		} else {
			int r = 1 << ( - 1 - numBits );
			if ( value > r - 1 ) {
				idLib::common->Warning( "idBitMsg::WriteBits: value overflow %d %d", value, numBits );
			} else if ( value < -r ) {
				idLib::common->Warning( "idBitMsg::WriteBits: value overflow %d %d", value, numBits );
			}
		}
	}

	if ( numBits < 0 ) {
		numBits = -numBits;
	}

	// check for msg overflow
	if ( CheckOverflow( numBits ) ) {
		return;
	}

	// write the bits
	while( numBits ) {
		if ( writeBit == 0 ) {
			writeData[curSize] = 0;
			curSize++;
		}
		put = 8 - writeBit;
		if ( put > numBits ) {
			put = numBits;
		}
		fraction = value & ( ( 1 << put ) - 1 );
		writeData[curSize - 1] |= fraction << writeBit;
		numBits -= put;
		value >>= put;
		writeBit = ( writeBit + put ) & 7;
	}
}

/*
================
idBitMsg::WriteString
================
*/
void idBitMsg::WriteString( const char *s, int maxLength, bool make7Bit ) {
	if ( !s ) {
		WriteData( "", 1 );
	} else {
		int i, l;
		byte *dataPtr;
		const byte *bytePtr;

		l = idStr::Length( s );
		if ( maxLength >= 0 && l >= maxLength ) {
			l = maxLength - 1;
		}
		dataPtr = GetByteSpace( l + 1 );
		bytePtr = reinterpret_cast<const byte *>(s);
		if ( make7Bit ) {
			for ( i = 0; i < l; i++ ) {
				if ( bytePtr[i] > 127 ) {
					dataPtr[i] = '.';
				} else {
					dataPtr[i] = bytePtr[i];
				}
			}
		} else {
			for ( i = 0; i < l; i++ ) {
				dataPtr[i] = bytePtr[i];
			}
		}
		dataPtr[i] = '\0';
	}
}

/*
================
idBitMsg::WriteData
================
*/
void idBitMsg::WriteData( const void *data, int length ) {
	memcpy( GetByteSpace( length ), data, length );
}

/*
================
idBitMsg::WriteNetadr
================
*/
void idBitMsg::WriteNetadr( const netadr_t adr ) {
	byte *dataPtr;
	dataPtr = GetByteSpace( 4 );
	memcpy( dataPtr, adr.ip, 4 );
	WriteUShort( adr.port );
}

/*
================
idBitMsg::WriteDelta
================
*/
void idBitMsg::WriteDelta( int oldValue, int newValue, int numBits ) {
	if ( oldValue == newValue ) {
		WriteBits( 0, 1 );
		return;
	}
	WriteBits( 1, 1 );
	WriteBits( newValue, numBits );
}

/*
================
idBitMsg::WriteDeltaByteCounter
================
*/
void idBitMsg::WriteDeltaByteCounter( int oldValue, int newValue ) {
	int i, x;

	x = oldValue ^ newValue;
	for ( i = 7; i > 0; i-- ) {
		if ( x & ( 1 << i ) ) {
			i++;
			break;
		}
	}
	WriteBits( i, 3 );
	if ( i ) {
		WriteBits( ( ( 1 << i ) - 1 ) & newValue, i );
	}
}

/*
================
idBitMsg::WriteDeltaShortCounter
================
*/
void idBitMsg::WriteDeltaShortCounter( int oldValue, int newValue ) {
	int i, x;

	x = oldValue ^ newValue;
	for ( i = 15; i > 0; i-- ) {
		if ( x & ( 1 << i ) ) {
			i++;
			break;
		}
	}
	WriteBits( i, 4 );
	if ( i ) {
		WriteBits( ( ( 1 << i ) - 1 ) & newValue, i );
	}
}

/*
================
idBitMsg::WriteDeltaLongCounter
================
*/
void idBitMsg::WriteDeltaLongCounter( int oldValue, int newValue ) {
	int i, x;

	x = oldValue ^ newValue;
	for ( i = 31; i > 0; i-- ) {
		if ( x & ( 1 << i ) ) {
			i++;
			break;
		}
	}
	WriteBits( i, 5 );
	if ( i ) {
		WriteBits( ( ( 1 << i ) - 1 ) & newValue, i );
	}
}

/*
==================
idBitMsg::WriteDeltaDict
==================
*/
bool idBitMsg::WriteDeltaDict( const idDict &dict, const idDict *base ) {
	int i;
	const idKeyValue *kv, *basekv;
	bool changed = false;

	if ( base != NULL ) {

		for ( i = 0; i < dict.GetNumKeyVals(); i++ ) {
			kv = dict.GetKeyVal( i );
			basekv = base->FindKey( kv->GetKey() );
			if ( basekv == NULL || basekv->GetValue().Icmp( kv->GetValue() ) != 0 ) {
				WriteString( kv->GetKey() );
				WriteString( kv->GetValue() );
				changed = true;
			}
		}

		WriteString( "" );

		for ( i = 0; i < base->GetNumKeyVals(); i++ ) {
			basekv = base->GetKeyVal( i );
			kv = dict.FindKey( basekv->GetKey() );
			if ( kv == NULL ) {
				WriteString( basekv->GetKey() );
				changed = true;
			}
		}

		WriteString( "" );

	} else {

		for ( i = 0; i < dict.GetNumKeyVals(); i++ ) {
			kv = dict.GetKeyVal( i );
			WriteString( kv->GetKey() );
			WriteString( kv->GetValue() );
			changed = true;
		}
		WriteString( "" );

		WriteString( "" );

	}

	return changed;
}

/*
================
idBitMsg::ReadBits

  If the number of bits is negative a sign is included.
================
*/
int idBitMsg::ReadBits( int numBits ) const {
	int		value;
	int		valueBits;
	int		get;
	int		fraction;
	bool	sgn;

	if ( !readData ) {
		idLib::common->FatalError( "idBitMsg::ReadBits: cannot read from message" );
	}

	// check if the number of bits is valid
	if ( numBits == 0 || numBits < -31 || numBits > 32 ) {
		idLib::common->FatalError( "idBitMsg::ReadBits: bad numBits %i", numBits );
	}

	value = 0;
	valueBits = 0;

	if ( numBits < 0 ) {
		numBits = -numBits;
		sgn = true;
	} else {
		sgn = false;
	}

	// check for overflow
	if ( numBits > GetRemainingReadBits() ) {
		return -1;
	}

	while ( valueBits < numBits ) {
		if ( readBit == 0 ) {
			readCount++;
		}
		get = 8 - readBit;
		if ( get > (numBits - valueBits) ) {
			get = numBits - valueBits;
		}
		fraction = readData[readCount - 1];
		fraction >>= readBit;
		fraction &= ( 1 << get ) - 1;
		value |= fraction << valueBits;

		valueBits += get;
		readBit = ( readBit + get ) & 7;
	}

	if ( sgn ) {
		if ( value & ( 1 << ( numBits - 1 ) ) ) {
			value |= -1 ^ ( ( 1 << numBits ) - 1 );
		}
	}

	return value;
}

/*
================
idBitMsg::ReadString
================
*/
int idBitMsg::ReadString( char *buffer, int bufferSize ) const {
	int	l, c;
	
	ReadByteAlign();
	l = 0;
	while( 1 ) {
		c = ReadByte();
		if ( c <= 0 || c >= 255 ) {
			break;
		}
		// translate all fmt spec to avoid crash bugs in string routines
		if ( c == '%' ) {
			c = '.';
		}

		// we will read past any excessively long string, so
		// the following data can be read, but the string will
		// be truncated
		if ( l < bufferSize - 1 ) {
			buffer[l] = c;
			l++;
		}
	}
	
	buffer[l] = 0;
	return l;
}

/*
================
idBitMsg::ReadData
================
*/
int idBitMsg::ReadData( void *data, int length ) const {
	int cnt;

	ReadByteAlign();
	cnt = readCount;

	if ( readCount + length > curSize ) {
		if ( data ) {
			memcpy( data, readData + readCount, GetRemaingData() );
		}
		readCount = curSize;
	} else {
		if ( data ) {
			memcpy( data, readData + readCount, length );
		}
		readCount += length;
	}

	return ( readCount - cnt );
}

/*
================
idBitMsg::ReadNetadr
================
*/
void idBitMsg::ReadNetadr( netadr_t *adr ) const {
	int i;
 
	adr->type = NA_IP;
	for ( i = 0; i < 4; i++ ) {
		adr->ip[ i ] = ReadByte();
	}
	adr->port = ReadUShort();
}

/*
================
idBitMsg::ReadDelta
================
*/
int idBitMsg::ReadDelta( int oldValue, int numBits ) const {
	if ( ReadBits( 1 ) ) {
		return ReadBits( numBits );
	}
	return oldValue;
}

/*
================
idBitMsg::ReadDeltaByteCounter
================
*/
int idBitMsg::ReadDeltaByteCounter( int oldValue ) const {
	int i, newValue;

	i = ReadBits( 3 );
	if ( !i ) {
		return oldValue;
	}
	newValue = ReadBits( i );
	return ( ( oldValue & ~( ( 1 << i ) - 1 ) ) | newValue );
}

/*
================
idBitMsg::ReadDeltaShortCounter
================
*/
int idBitMsg::ReadDeltaShortCounter( int oldValue ) const {
	int i, newValue;

	i = ReadBits( 4 );
	if ( !i ) {
		return oldValue;
	}
	newValue = ReadBits( i );
	return ( ( oldValue & ~( ( 1 << i ) - 1 )) | newValue );
}

/*
================
idBitMsg::ReadDeltaLongCounter
================
*/
int idBitMsg::ReadDeltaLongCounter( int oldValue ) const {
	int i, newValue;

	i = ReadBits( 5 );
	if ( !i ) {
		return oldValue;
	}
	newValue = ReadBits( i );
	return ( ( oldValue & ~( ( 1 << i ) - 1 ) ) | newValue );
}

/*
==================
idBitMsg::ReadDeltaDict
==================
*/
bool idBitMsg::ReadDeltaDict( idDict &dict, const idDict *base ) const {
	char		key[MAX_STRING_CHARS];
	char		value[MAX_STRING_CHARS];
	bool		changed = false;

	if ( base != NULL ) {
		dict = *base;
	} else {
		dict.Clear();
	}

	while( ReadString( key, sizeof( key ) ) != 0 ) {
		ReadString( value, sizeof( value ) );
		dict.Set( key, value );
		changed = true;
	}

	while( ReadString( key, sizeof( key ) ) != 0 ) {
		dict.Delete( key );
		changed = true;
	}

	return changed;
}

/*
================
idBitMsg::DirToBits
================
*/
int idBitMsg::DirToBits( const idVec3 &dir, int numBits ) {
	int max, bits;
	float bias;

	assert( numBits >= 6 && numBits <= 32 );
	assert( dir.LengthSqr() - 1.0f < 0.01f );

	numBits /= 3;
	max = ( 1 << ( numBits - 1 ) ) - 1;
	bias = 0.5f / max;

	bits = FLOATSIGNBITSET( dir.x ) << ( numBits * 3 - 1 );
	bits |= ( idMath::Ftoi( ( idMath::Fabs( dir.x ) + bias ) * max ) ) << ( numBits * 2 );
	bits |= FLOATSIGNBITSET( dir.y ) << ( numBits * 2 - 1 );
	bits |= ( idMath::Ftoi( ( idMath::Fabs( dir.y ) + bias ) * max ) ) << ( numBits * 1 );
	bits |= FLOATSIGNBITSET( dir.z ) << ( numBits * 1 - 1 );
	bits |= ( idMath::Ftoi( ( idMath::Fabs( dir.z ) + bias ) * max ) ) << ( numBits * 0 );
	return bits;
}

/*
================
idBitMsg::BitsToDir
================
*/
idVec3 idBitMsg::BitsToDir( int bits, int numBits ) {
	static float sign[2] = { 1.0f, -1.0f };
	int max;
	float invMax;
	idVec3 dir;

	assert( numBits >= 6 && numBits <= 32 );

	numBits /= 3;
	max = ( 1 << ( numBits - 1 ) ) - 1;
	invMax = 1.0f / max;

	dir.x = sign[( bits >> ( numBits * 3 - 1 ) ) & 1] * ( ( bits >> ( numBits * 2 ) ) & max ) * invMax;
	dir.y = sign[( bits >> ( numBits * 2 - 1 ) ) & 1] * ( ( bits >> ( numBits * 1 ) ) & max ) * invMax;
	dir.z = sign[( bits >> ( numBits * 1 - 1 ) ) & 1] * ( ( bits >> ( numBits * 0 ) ) & max ) * invMax;
	dir.NormalizeFast();
	return dir;
}


/*
==============================================================================

  idBitMsgDelta

==============================================================================
*/

const int MAX_DATA_BUFFER		= 1024;

/*
================
idBitMsgDelta::WriteBits
================
*/
void idBitMsgDelta::WriteBits( int value, int numBits ) {
	if ( newBase ) {
		newBase->WriteBits( value, numBits );
	}

	if ( !base ) {
		writeDelta->WriteBits( value, numBits );
		changed = true;
	} else {
		int baseValue = base->ReadBits( numBits );
		if ( baseValue == value ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteBits( value, numBits );
			changed = true;
		}
	}
}

/*
================
idBitMsgDelta::WriteDelta
================
*/
void idBitMsgDelta::WriteDelta( int oldValue, int newValue, int numBits ) {
	if ( newBase ) {
		newBase->WriteBits( newValue, numBits );
	}

	if ( !base ) {
		if ( oldValue == newValue ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteBits( newValue, numBits );
		}
		changed = true;
	} else {
		int baseValue = base->ReadBits( numBits );
		if ( baseValue == newValue ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			if ( oldValue == newValue ) {
				writeDelta->WriteBits( 0, 1 );
				changed = true;
			} else {
				writeDelta->WriteBits( 1, 1 );
				writeDelta->WriteBits( newValue, numBits );
				changed = true;
			}
		}
	}
}

/*
================
idBitMsgDelta::ReadBits
================
*/
int idBitMsgDelta::ReadBits( int numBits ) const {
	int value;

	if ( !base ) {
		value = readDelta->ReadBits( numBits );
		changed = true;
	} else {
		int baseValue = base->ReadBits( numBits );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			value = baseValue;
		} else {
			value = readDelta->ReadBits( numBits );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteBits( value, numBits );
	}
	return value;
}

/*
================
idBitMsgDelta::ReadDelta
================
*/
int idBitMsgDelta::ReadDelta( int oldValue, int numBits ) const {
	int value;

	if ( !base ) {
		if ( readDelta->ReadBits( 1 ) == 0 ) {
			value = oldValue;
		} else {
			value = readDelta->ReadBits( numBits );
		}
		changed = true;
	} else {
		int baseValue = base->ReadBits( numBits );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			value = baseValue;
		} else if ( readDelta->ReadBits( 1 ) == 0 ) {
			value = oldValue;
			changed = true;
		} else {
			value = readDelta->ReadBits( numBits );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteBits( value, numBits );
	}
	return value;
}

/*
================
idBitMsgDelta::WriteString
================
*/
void idBitMsgDelta::WriteString( const char *s, int maxLength ) {
	if ( newBase ) {
		newBase->WriteString( s, maxLength );
	}

	if ( !base ) {
		writeDelta->WriteString( s, maxLength );
		changed = true;
	} else {
		char baseString[MAX_DATA_BUFFER];
		base->ReadString( baseString, sizeof( baseString ) );
		if ( idStr::Cmp( s, baseString ) == 0 ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteString( s, maxLength );
			changed = true;
		}
	}
}

/*
================
idBitMsgDelta::WriteData
================
*/
void idBitMsgDelta::WriteData( const void *data, int length ) {
	if ( newBase ) {
		newBase->WriteData( data, length );
	}

	if ( !base ) {
		writeDelta->WriteData( data, length );
		changed = true;
	} else {
		byte baseData[MAX_DATA_BUFFER];
		assert( length < sizeof( baseData ) );
		base->ReadData( baseData, length );
		if ( memcmp( data, baseData, length ) == 0 ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteData( data, length );
			changed = true;
		}
	}
}

/*
================
idBitMsgDelta::WriteDict
================
*/
void idBitMsgDelta::WriteDict( const idDict &dict ) {
	if ( newBase ) {
		newBase->WriteDeltaDict( dict, NULL );
	}

	if ( !base ) {
		writeDelta->WriteDeltaDict( dict, NULL );
		changed = true;
	} else {
		idDict baseDict;
		base->ReadDeltaDict( baseDict, NULL );
		changed = writeDelta->WriteDeltaDict( dict, &baseDict );
	}
}

/*
================
idBitMsgDelta::WriteDeltaByteCounter
================
*/
void idBitMsgDelta::WriteDeltaByteCounter( int oldValue, int newValue ) {
	if ( newBase ) {
		newBase->WriteBits( newValue, 8 );
	}

	if ( !base ) {
		writeDelta->WriteDeltaByteCounter( oldValue, newValue );
		changed = true;
	} else {
		int baseValue = base->ReadBits( 8 );
		if ( baseValue == newValue ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteDeltaByteCounter( oldValue, newValue );
			changed = true;
		}
	}
}

/*
================
idBitMsgDelta::WriteDeltaShortCounter
================
*/
void idBitMsgDelta::WriteDeltaShortCounter( int oldValue, int newValue ) {
	if ( newBase ) {
		newBase->WriteBits( newValue, 16 );
	}

	if ( !base ) {
		writeDelta->WriteDeltaShortCounter( oldValue, newValue );
		changed = true;
	} else {
		int baseValue = base->ReadBits( 16 );
		if ( baseValue == newValue ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteDeltaShortCounter( oldValue, newValue );
			changed = true;
		}
	}
}

/*
================
idBitMsgDelta::WriteDeltaLongCounter
================
*/
void idBitMsgDelta::WriteDeltaLongCounter( int oldValue, int newValue ) {
	if ( newBase ) {
		newBase->WriteBits( newValue, 32 );
	}

	if ( !base ) {
		writeDelta->WriteDeltaLongCounter( oldValue, newValue );
		changed = true;
	} else {
		int baseValue = base->ReadBits( 32 );
		if ( baseValue == newValue ) {
			writeDelta->WriteBits( 0, 1 );
		} else {
			writeDelta->WriteBits( 1, 1 );
			writeDelta->WriteDeltaLongCounter( oldValue, newValue );
			changed = true;
		}
	}
}

/*
================
idBitMsgDelta::ReadString
================
*/
void idBitMsgDelta::ReadString( char *buffer, int bufferSize ) const {
	if ( !base ) {
		readDelta->ReadString( buffer, bufferSize );
		changed = true;
	} else {
		char baseString[MAX_DATA_BUFFER];
		base->ReadString( baseString, sizeof( baseString ) );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			idStr::Copynz( buffer, baseString, bufferSize );
		} else {
			readDelta->ReadString( buffer, bufferSize );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteString( buffer );
	}
}

/*
================
idBitMsgDelta::ReadData
================
*/
void idBitMsgDelta::ReadData( void *data, int length ) const {
	if ( !base ) {
		readDelta->ReadData( data, length );
		changed = true;
	} else {
		char baseData[MAX_DATA_BUFFER];
		assert( length < sizeof( baseData ) );
		base->ReadData( baseData, length );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			memcpy( data, baseData, length );
		} else {
			readDelta->ReadData( data, length );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteData( data, length );
	}
}

/*
================
idBitMsgDelta::ReadDict
================
*/
void idBitMsgDelta::ReadDict( idDict &dict ) {
	if ( !base ) {
		readDelta->ReadDeltaDict( dict, NULL );
		changed = true;
	} else {
		idDict baseDict;
		base->ReadDeltaDict( baseDict, NULL );
		if ( !readDelta ) {
			dict = baseDict;
		} else {
			changed = readDelta->ReadDeltaDict( dict, &baseDict );
		}
	}

	if ( newBase ) {
		newBase->WriteDeltaDict( dict, NULL );
	}
}

/*
================
idBitMsgDelta::ReadDeltaByteCounter
================
*/
int idBitMsgDelta::ReadDeltaByteCounter( int oldValue ) const {
	int value;

	if ( !base ) {
		value = readDelta->ReadDeltaByteCounter( oldValue );
		changed = true;
	} else {
		int baseValue = base->ReadBits( 8 );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			value = baseValue;
		} else {
			value = readDelta->ReadDeltaByteCounter( oldValue );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteBits( value, 8 );
	}
	return value;
}

/*
================
idBitMsgDelta::ReadDeltaShortCounter
================
*/
int idBitMsgDelta::ReadDeltaShortCounter( int oldValue ) const {
	int value;

	if ( !base ) {
		value = readDelta->ReadDeltaShortCounter( oldValue );
		changed = true;
	} else {
		int baseValue = base->ReadBits( 16 );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			value = baseValue;
		} else {
			value = readDelta->ReadDeltaShortCounter( oldValue );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteBits( value, 16 );
	}
	return value;
}

/*
================
idBitMsgDelta::ReadDeltaLongCounter
================
*/
int idBitMsgDelta::ReadDeltaLongCounter( int oldValue ) const {
	int value;

	if ( !base ) {
		value = readDelta->ReadDeltaLongCounter( oldValue );
		changed = true;
	} else {
		int baseValue = base->ReadBits( 32 );
		if ( !readDelta || readDelta->ReadBits( 1 ) == 0 ) {
			value = baseValue;
		} else {
			value = readDelta->ReadDeltaLongCounter( oldValue );
			changed = true;
		}
	}

	if ( newBase ) {
		newBase->WriteBits( value, 32 );
	}
	return value;
}

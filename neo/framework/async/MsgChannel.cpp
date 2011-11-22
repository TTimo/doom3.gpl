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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "MsgChannel.h"

/*

packet header
-------------
2 bytes		id
4 bytes		outgoing sequence. high bit will be set if this is a fragmented message.
2 bytes		optional fragment start byte if fragment bit is set.
2 bytes		optional fragment length if fragment bit is set. if < FRAGMENT_SIZE, this is the last fragment.

If the id is -1, the packet should be handled as an out-of-band
message instead of as part of the message channel.

All fragments will have the same sequence numbers.

*/


#define	MAX_PACKETLEN			1400		// max size of a network packet
#define	FRAGMENT_SIZE			(MAX_PACKETLEN - 100)
#define	FRAGMENT_BIT			(1<<31)

idCVar net_channelShowPackets( "net_channelShowPackets", "0", CVAR_SYSTEM | CVAR_BOOL, "show all packets" );
idCVar net_channelShowDrop( "net_channelShowDrop", "0", CVAR_SYSTEM | CVAR_BOOL, "show dropped packets" );

/*
===============
idMsgQueue::idMsgQueue
===============
*/
idMsgQueue::idMsgQueue( void ) {
	Init( 0 );
}

/*
===============
idMsgQueue::Init
===============
*/
void idMsgQueue::Init( int sequence ) {
	first = last = sequence;
	startIndex = endIndex = 0;
}

/*
===============
idMsgQueue::Add
===============
*/
bool idMsgQueue::Add( const byte *data, const int size ) {
	if ( GetSpaceLeft() < size + 8 ) {
		return false;
	}
	int sequence = last;
	WriteShort( size );
	WriteLong( sequence );
	WriteData( data, size );
	last++;
	return true;
}

/*
===============
idMsgQueue::Get
===============
*/
bool idMsgQueue::Get( byte *data, int &size ) {
	if ( first == last ) {
		size = 0;
		return false;
	}
	int sequence;
	size = ReadShort();
	sequence = ReadLong();
	ReadData( data, size );
	assert( sequence == first );
	first++;
	return true;
}

/*
===============
idMsgQueue::GetTotalSize
===============
*/
int idMsgQueue::GetTotalSize( void ) const {
	if ( startIndex <= endIndex ) {
		return ( endIndex - startIndex );
	} else {
		return ( sizeof( buffer ) - startIndex + endIndex );
	}
}

/*
===============
idMsgQueue::GetSpaceLeft
===============
*/
int idMsgQueue::GetSpaceLeft( void ) const {
	if ( startIndex <= endIndex ) {
		return sizeof( buffer ) - ( endIndex - startIndex ) - 1;
	} else {
		return ( startIndex - endIndex ) - 1;
	}
}

/*
===============
idMsgQueue::CopyToBuffer
===============
*/
void idMsgQueue::CopyToBuffer( byte *buf ) const {
	if ( startIndex <= endIndex ) {
		memcpy( buf, buffer + startIndex, endIndex - startIndex );
	} else {
		memcpy( buf, buffer + startIndex, sizeof( buffer ) - startIndex );
		memcpy( buf + sizeof( buffer ) - startIndex, buffer, endIndex );
	}
}

/*
===============
idMsgQueue::WriteByte
===============
*/
void idMsgQueue::WriteByte( byte b ) {
	buffer[endIndex] = b;
	endIndex = ( endIndex + 1 ) & ( MAX_MSG_QUEUE_SIZE - 1 );
}

/*
===============
idMsgQueue::ReadByte
===============
*/
byte idMsgQueue::ReadByte( void ) {
	byte b = buffer[startIndex];
	startIndex = ( startIndex + 1 ) & ( MAX_MSG_QUEUE_SIZE - 1 );
	return b;
}

/*
===============
idMsgQueue::WriteShort
===============
*/
void idMsgQueue::WriteShort( int s ) {
	WriteByte( ( s >>  0 ) & 255 );
	WriteByte( ( s >>  8 ) & 255 );
}

/*
===============
idMsgQueue::ReadShort
===============
*/
int idMsgQueue::ReadShort( void ) {
	return ReadByte() | ( ReadByte() << 8 );
}

/*
===============
idMsgQueue::WriteLong
===============
*/
void idMsgQueue::WriteLong( int l ) {
	WriteByte( ( l >>  0 ) & 255 );
	WriteByte( ( l >>  8 ) & 255 );
	WriteByte( ( l >> 16 ) & 255 );
	WriteByte( ( l >> 24 ) & 255 );
}

/*
===============
idMsgQueue::ReadLong
===============
*/
int idMsgQueue::ReadLong( void ) {
	return ReadByte() | ( ReadByte() << 8 ) | ( ReadByte() << 16 ) | ( ReadByte() << 24 );
}

/*
===============
idMsgQueue::WriteData
===============
*/
void idMsgQueue::WriteData( const byte *data, const int size ) {
	for ( int i = 0; i < size; i++ ) {
		WriteByte( data[i] );
	}
}

/*
===============
idMsgQueue::ReadData
===============
*/
void idMsgQueue::ReadData( byte *data, const int size ) {
	if ( data ) {
		for ( int i = 0; i < size; i++ ) {
			data[i] = ReadByte();
		}
	} else {
		for ( int i = 0; i < size; i++ ) {
			ReadByte();
		}
	}
}


/*
===============
idMsgChannel::idMsgChannel
===============
*/
idMsgChannel::idMsgChannel() {
	id = -1;
}

/*
==============
idMsgChannel::Init

  Opens a channel to a remote system.
==============
*/
void idMsgChannel::Init( const netadr_t adr, const int id ) {
	this->remoteAddress = adr;
	this->id = id;
	this->maxRate = 50000;
	this->compressor = idCompressor::AllocRunLength_ZeroBased();

	lastSendTime = 0;
	lastDataBytes = 0;
	outgoingRateTime = 0;
	outgoingRateBytes = 0;
	incomingRateTime = 0;
	incomingRateBytes = 0;
	incomingReceivedPackets = 0.0f;
	incomingDroppedPackets = 0.0f;
	incomingPacketLossTime = 0;
	outgoingCompression = 0.0f;
	incomingCompression = 0.0f;
	outgoingSequence = 1;
	incomingSequence = 0;
	unsentFragments = false;
	unsentFragmentStart = 0;
	fragmentSequence = 0;
	fragmentLength = 0;
	reliableSend.Init( 1 );
	reliableReceive.Init( 0 );
}

/*
===============
idMsgChannel::Shutdown
================
*/
void idMsgChannel::Shutdown( void ) {
	delete compressor;
	compressor = NULL;
}

/*
=================
idMsgChannel::ResetRate
=================
*/
void idMsgChannel::ResetRate( void ) {
	lastSendTime = 0;
	lastDataBytes = 0;
	outgoingRateTime = 0;
	outgoingRateBytes = 0;
	incomingRateTime = 0;
	incomingRateBytes = 0;
}

/*
=================
idMsgChannel::ReadyToSend
=================
*/
bool idMsgChannel::ReadyToSend( const int time ) const {
	int deltaTime;

	if ( !maxRate ) {
		return true;
	}
	deltaTime = time - lastSendTime;
	if ( deltaTime > 1000 ) {
		return true;
	}
	return ( ( lastDataBytes - ( deltaTime * maxRate ) / 1000 ) <= 0 );
}

/*
===============
idMsgChannel::WriteMessageData
================
*/
void idMsgChannel::WriteMessageData( idBitMsg &out, const idBitMsg &msg ) {
	idBitMsg tmp;
	byte tmpBuf[MAX_MESSAGE_SIZE];

	tmp.Init( tmpBuf, sizeof( tmpBuf ) );

	// write acknowledgement of last received reliable message
	tmp.WriteLong( reliableReceive.GetLast() );

	// write reliable messages
	reliableSend.CopyToBuffer( tmp.GetData() + tmp.GetSize() );
	tmp.SetSize( tmp.GetSize() + reliableSend.GetTotalSize() );
	tmp.WriteShort( 0 );

	// write data
	tmp.WriteData( msg.GetData(), msg.GetSize() );

	// write message size
	out.WriteShort( tmp.GetSize() );

	// compress message
	idFile_BitMsg file( out );
	compressor->Init( &file, true, 3 );
	compressor->Write( tmp.GetData(), tmp.GetSize() );
	compressor->FinishCompress();
	outgoingCompression = compressor->GetCompressionRatio();
}

/*
===============
idMsgChannel::ReadMessageData
================
*/
bool idMsgChannel::ReadMessageData( idBitMsg &out, const idBitMsg &msg ) {
	int reliableAcknowledge, reliableMessageSize, reliableSequence;

	// read message size
	out.SetSize( msg.ReadShort() );

	// decompress message
	idFile_BitMsg file( msg );
	compressor->Init( &file, false, 3 );
	compressor->Read( out.GetData(), out.GetSize() );
	incomingCompression = compressor->GetCompressionRatio();
	out.BeginReading();

	// read acknowledgement of sent reliable messages
	reliableAcknowledge = out.ReadLong();

	// remove acknowledged reliable messages
	while( reliableSend.GetFirst() <= reliableAcknowledge ) {
		if ( !reliableSend.Get( NULL, reliableMessageSize ) ) {
			break;
		}
	}

	// read reliable messages
	reliableMessageSize = out.ReadShort();
	while( reliableMessageSize != 0 ) {
		if ( reliableMessageSize <= 0 || reliableMessageSize > out.GetSize() - out.GetReadCount() ) {
			common->Printf( "%s: bad reliable message\n", Sys_NetAdrToString( remoteAddress ) );
			return false;
		}
		reliableSequence = out.ReadLong();
		if ( reliableSequence == reliableReceive.GetLast() + 1 ) {
			reliableReceive.Add( out.GetData() + out.GetReadCount(), reliableMessageSize );
		}
		out.ReadData( NULL, reliableMessageSize );
		reliableMessageSize = out.ReadShort();
	}

	return true;
}

/*
=================
idMsgChannel::SendNextFragment

  Sends one fragment of the current message.
=================
*/
void idMsgChannel::SendNextFragment( idPort &port, const int time ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_PACKETLEN];
	int			fragLength;

	if ( remoteAddress.type == NA_BAD ) {
		return;
	}

	if ( !unsentFragments ) {
		return;
	}

	// write the packet
	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( id );
	msg.WriteLong( outgoingSequence | FRAGMENT_BIT );

	fragLength = FRAGMENT_SIZE;
	if ( unsentFragmentStart + fragLength > unsentMsg.GetSize() ) {
		fragLength = unsentMsg.GetSize() - unsentFragmentStart;
	}

	msg.WriteShort( unsentFragmentStart );
	msg.WriteShort( fragLength );
	msg.WriteData( unsentMsg.GetData() + unsentFragmentStart, fragLength );

	// send the packet
	port.SendPacket( remoteAddress, msg.GetData(), msg.GetSize() );

	// update rate control variables
	UpdateOutgoingRate( time, msg.GetSize() );

	if ( net_channelShowPackets.GetBool() ) {
		common->Printf( "%d send %4i : s = %i fragment = %i,%i\n", id, msg.GetSize(), outgoingSequence - 1, unsentFragmentStart, fragLength );
	}

	unsentFragmentStart += fragLength;

	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if ( unsentFragmentStart == unsentMsg.GetSize() && fragLength != FRAGMENT_SIZE ) {
		outgoingSequence++;
		unsentFragments = false;
	}
}

/*
===============
idMsgChannel::SendMessage

  Sends a message to a connection, fragmenting if necessary
  A 0 length will still generate a packet.
================
*/
int idMsgChannel::SendMessage( idPort &port, const int time, const idBitMsg &msg ) {
	int totalLength;

	if ( remoteAddress.type == NA_BAD ) {
		return -1;
	}

	if ( unsentFragments ) {
		common->Error( "idMsgChannel::SendMessage: called with unsent fragments left" );
		return -1;
	}

	totalLength = 4 + reliableSend.GetTotalSize() + 4 + msg.GetSize();

	if ( totalLength > MAX_MESSAGE_SIZE ) {
		common->Printf( "idMsgChannel::SendMessage: message too large, length = %i\n", totalLength );
		return -1;
	}

	unsentMsg.Init( unsentBuffer, sizeof( unsentBuffer ) );
	unsentMsg.BeginWriting();

	// fragment large messages
	if ( totalLength >= FRAGMENT_SIZE ) {
		unsentFragments = true;
		unsentFragmentStart = 0;

		// write out the message data
		WriteMessageData( unsentMsg, msg );

		// send the first fragment now
		SendNextFragment( port, time );

		return outgoingSequence;
	}

	// write the header
	unsentMsg.WriteShort( id );
	unsentMsg.WriteLong( outgoingSequence );

	// write out the message data
	WriteMessageData( unsentMsg, msg );

	// send the packet
	port.SendPacket( remoteAddress, unsentMsg.GetData(), unsentMsg.GetSize() );

	// update rate control variables
	UpdateOutgoingRate( time, unsentMsg.GetSize() );

	if ( net_channelShowPackets.GetBool() ) {
		common->Printf( "%d send %4i : s = %i ack = %i\n", id, unsentMsg.GetSize(), outgoingSequence - 1, incomingSequence );
	}

	outgoingSequence++;

	return ( outgoingSequence - 1 );
}

/*
=================
idMsgChannel::Process

  Returns false if the message should not be processed due to being out of order or a fragment.

  msg must be large enough to hold MAX_MESSAGE_SIZE, because if this is the final
  fragment of a multi-part message, the entire thing will be copied out.
=================
*/
bool idMsgChannel::Process( const netadr_t from, int time, idBitMsg &msg, int &sequence ) {
	int			fragStart, fragLength, dropped;
	bool		fragmented;
	idBitMsg	fragMsg;

	// the IP port can't be used to differentiate them, because
	// some address translating routers periodically change UDP
	// port assignments
	if ( remoteAddress.port != from.port ) {
		common->Printf( "idMsgChannel::Process: fixing up a translated port\n" );
		remoteAddress.port = from.port;
	}

	// update incoming rate
	UpdateIncomingRate( time, msg.GetSize() );

	// get sequence numbers
	sequence = msg.ReadLong();

	// check for fragment information
	if ( sequence & FRAGMENT_BIT ) {
		sequence &= ~FRAGMENT_BIT;
		fragmented = true;
	} else {
		fragmented = false;
	}

	// read the fragment information
	if ( fragmented ) {
		fragStart = msg.ReadShort();
		fragLength = msg.ReadShort();
	} else {
		fragStart = 0;		// stop warning message
		fragLength = 0;
	}

	if ( net_channelShowPackets.GetBool() ) {
		if ( fragmented ) {
			common->Printf( "%d recv %4i : s = %i fragment = %i,%i\n", id, msg.GetSize(), sequence, fragStart, fragLength );
		} else {
			common->Printf( "%d recv %4i : s = %i\n", id, msg.GetSize(), sequence );
		}
	}

	//
	// discard out of order or duplicated packets
	//
	if ( sequence <= incomingSequence ) {
		if ( net_channelShowDrop.GetBool() || net_channelShowPackets.GetBool() ) {
			common->Printf( "%s: out of order packet %i at %i\n", Sys_NetAdrToString( remoteAddress ),  sequence, incomingSequence );
		}
		return false;
	}

	//
	// dropped packets don't keep this message from being used
	//
	dropped = sequence - (incomingSequence+1);
	if ( dropped > 0 ) {
		if ( net_channelShowDrop.GetBool() || net_channelShowPackets.GetBool() ) {
			common->Printf( "%s: dropped %i packets at %i\n", Sys_NetAdrToString( remoteAddress ), dropped, sequence );
		}
		UpdatePacketLoss( time, 0, dropped );
	}

	//
	// if the message is fragmented
	//
	if ( fragmented ) {
		// make sure we have the correct sequence number
		if ( sequence != fragmentSequence ) {
			fragmentSequence = sequence;
			fragmentLength = 0;
		}

		// if we missed a fragment, dump the message
		if ( fragStart != fragmentLength ) {
			if ( net_channelShowDrop.GetBool() || net_channelShowPackets.GetBool() ) {
				common->Printf( "%s: dropped a message fragment at seq %d\n", Sys_NetAdrToString( remoteAddress ), sequence );
			}
			// we can still keep the part that we have so far,
			// so we don't need to clear fragmentLength
			UpdatePacketLoss( time, 0, 1 );
			return false;
		}

		// copy the fragment to the fragment buffer
		if ( fragLength < 0 || fragLength > msg.GetRemaingData() || fragmentLength + fragLength > sizeof( fragmentBuffer ) ) {
			if ( net_channelShowDrop.GetBool() || net_channelShowPackets.GetBool() ) {
				common->Printf( "%s: illegal fragment length\n", Sys_NetAdrToString( remoteAddress ) );
			}
			UpdatePacketLoss( time, 0, 1 );
			return false;
		}

		memcpy( fragmentBuffer + fragmentLength, msg.GetData() + msg.GetReadCount(), fragLength );

		fragmentLength += fragLength;

		UpdatePacketLoss( time, 1, 0 );

		// if this wasn't the last fragment, don't process anything
		if ( fragLength == FRAGMENT_SIZE ) {
			return false;
		}

	} else {
		memcpy( fragmentBuffer, msg.GetData() + msg.GetReadCount(), msg.GetRemaingData() );
		fragmentLength = msg.GetRemaingData();
		UpdatePacketLoss( time, 1, 0 );
	}

	fragMsg.Init( fragmentBuffer, fragmentLength );
	fragMsg.SetSize( fragmentLength );
	fragMsg.BeginReading();

	incomingSequence = sequence;

	// read the message data
	if ( !ReadMessageData( msg, fragMsg ) ) {
		return false;
	}

	return true;
}

/*
=================
idMsgChannel::SendReliableMessage
=================
*/
bool idMsgChannel::SendReliableMessage( const idBitMsg &msg ) {
	bool result;

	assert( remoteAddress.type != NA_BAD );
	if ( remoteAddress.type == NA_BAD ) {
		return false;
	}
	result = reliableSend.Add( msg.GetData(), msg.GetSize() );
	if ( !result ) {
		common->Warning( "idMsgChannel::SendReliableMessage: overflowed" );
		return false;
	}
	return result;
}

/*
=================
idMsgChannel::GetReliableMessage
=================
*/
bool idMsgChannel::GetReliableMessage( idBitMsg &msg ) {
	int size;
	bool result;

	result = reliableReceive.Get( msg.GetData(), size );
	msg.SetSize( size );
	msg.BeginReading();
	return result;
}

/*
===============
idMsgChannel::ClearReliableMessages
================
*/
void idMsgChannel::ClearReliableMessages( void ) {
	reliableSend.Init( 1 );
	reliableReceive.Init( 0 );
}

/*
=================
idMsgChannel::UpdateOutgoingRate
=================
*/
void idMsgChannel::UpdateOutgoingRate( const int time, const int size ) {
	// update the outgoing rate control variables
	int deltaTime = time - lastSendTime;
	if ( deltaTime > 1000 ) {
		lastDataBytes = 0;
	} else {
		lastDataBytes -= ( deltaTime * maxRate ) / 1000;
		if ( lastDataBytes < 0 ) {
			lastDataBytes = 0;
		}
	}
	lastDataBytes += size;
	lastSendTime = time;

	// update outgoing rate variables
	if ( time - outgoingRateTime > 1000 ) {
		outgoingRateBytes -= outgoingRateBytes * ( time - outgoingRateTime - 1000 ) / 1000;
		if ( outgoingRateBytes < 0 ) {
			outgoingRateBytes = 0;
		}
	}
	outgoingRateTime = time - 1000;
	outgoingRateBytes += size;
}

/*
=================
idMsgChannel::UpdateIncomingRate
=================
*/
void idMsgChannel::UpdateIncomingRate( const int time, const int size ) {
	// update incoming rate variables
	if ( time - incomingRateTime > 1000 ) {
		incomingRateBytes -= incomingRateBytes * ( time - incomingRateTime - 1000 ) / 1000;
		if ( incomingRateBytes < 0 ) {
			incomingRateBytes = 0;
		}
	}
	incomingRateTime = time - 1000;
	incomingRateBytes += size;
}

/*
=================
idMsgChannel::UpdatePacketLoss
=================
*/
void idMsgChannel::UpdatePacketLoss( const int time, const int numReceived, const int numDropped ) {
	// update incoming packet loss variables
	if ( time - incomingPacketLossTime > 5000 ) {
		float scale = ( time - incomingPacketLossTime - 5000 ) * ( 1.0f / 5000.0f );
		incomingReceivedPackets -= incomingReceivedPackets * scale;
		if ( incomingReceivedPackets < 0.0f ) {
			incomingReceivedPackets = 0.0f;
		}
		incomingDroppedPackets -= incomingDroppedPackets * scale;
		if ( incomingDroppedPackets < 0.0f ) {
			incomingDroppedPackets = 0.0f;
		}
	}
	incomingPacketLossTime = time - 5000;
	incomingReceivedPackets += numReceived;
	incomingDroppedPackets += numDropped;
}

/*
=================
idMsgChannel::GetIncomingPacketLoss
=================
*/
float idMsgChannel::GetIncomingPacketLoss( void ) const {
	if ( incomingReceivedPackets == 0.0f && incomingDroppedPackets == 0.0f ) {
		return 0.0f;
	}
	return incomingDroppedPackets * 100.0f / ( incomingReceivedPackets + incomingDroppedPackets );
}

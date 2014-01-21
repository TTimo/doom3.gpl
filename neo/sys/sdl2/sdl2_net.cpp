#include "../../idlib/precompiled.h"
#pragma hdrstop

void Sys_InitNetworking( void )
{
}

bool Sys_IsLANAddress( const netadr_t adr )
{
	return false;
}

const char *Sys_NetAdrToString( const netadr_t a )
{
	return NULL;
}

/*
=============
Sys_StringToNetAdr
=============
*/
bool Sys_StringToNetAdr( const char *s, netadr_t *a, bool doDNSResolve )
{
	return false;
}

/*
 ===================
 Sys_CompareNetAdrBase
 
 Compares without the port
 ===================
 */
bool Sys_CompareNetAdrBase( const netadr_t a, const netadr_t b )
{
}

/*
 ==================
 idPort::idPort
 ==================
 */
idPort::idPort()
{
}

/*
 ==================
 idPort::~idPort
 ==================
 */
idPort::~idPort()
{
}

/*
 ==================
 InitForPort
 ==================
 */
bool idPort::InitForPort( int portNumber )
{
	return true;
}

/*
 ==================
 idPort::Close
 ==================
 */
void idPort::Close()
{
}

/*
 ==================
 idPort::GetPacket
 ==================
 */
bool idPort::GetPacket( netadr_t &from, void *data, int &size, int maxSize )
{
}

/*
 ==================
 idPort::GetPacketBlocking
 ==================
 */
bool idPort::GetPacketBlocking( netadr_t &from, void *data, int &size, int maxSize, int timeout )
{
}

/*
 ==================
 idPort::SendPacket
 ==================
 */
void idPort::SendPacket( const netadr_t to, const void *data, int size )
{
}

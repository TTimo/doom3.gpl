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

#include <stdio.h>
#include <sys/time.h>
#include <sched.h>
#include <errno.h>

/*
================
Sys_Milliseconds
================
*/
/* base time in seconds, that's our origin
   timeval:tv_sec is an int: 
   assuming this wraps every 0x7fffffff - ~68 years since the Epoch (1970) - we're safe till 2038
   using unsigned long data type to work right with Sys_XTimeToSysTime */
unsigned long sys_timeBase = 0;
/* current time in ms, using sys_timeBase as origin
   NOTE: sys_timeBase*1000 + curtime -> ms since the Epoch
     0x7fffffff ms - ~24 days
		 or is it 48 days? the specs say int, but maybe it's casted from unsigned int?
*/
int Sys_Milliseconds(void)
{
	int curtime;
	struct timeval tp;

	gettimeofday(&tp, NULL);

	if (!sys_timeBase) {
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec / 1000;
	}

	curtime = (tp.tv_sec - sys_timeBase) * 1000 + tp.tv_usec / 1000;

	return curtime;
}

#define STAT_BUF 100

int main(int argc, void *argv[]) {
	int start = 30; // start waiting with 30 ms 
	int dec = 2; // decrement by 2 ms
	int min = 4; // min wait test
	int i, j, now, next;
	int stats[STAT_BUF];

	struct sched_param parm;
	
	Sys_Milliseconds(); // init

	// set schedule policy to see if that affects usleep
	// (root rights required for that)
	parm.sched_priority = 99;
	if ( sched_setscheduler(0, SCHED_RR, &parm) != 0 ) {
		printf("sched_setscheduler SCHED_RR failed: %s\n", strerror(errno) );
	} else {
		printf("sched_setscheduler SCHED_RR ok\n");
	}
	
	// now run the test
	for( i = start ; i >= min ; i -= dec ) {
		printf( "sleep %d ms", i );
		for( j = 0 ; j < STAT_BUF ; j++ ) {
			now = Sys_Milliseconds();
			usleep(i*1000);			
			stats[j] = Sys_Milliseconds() - now;
		}
		for( j = 0; j < STAT_BUF; j++) {
			if ( ! (j & 0xf) ) {
				printf("\n");
			}
			printf( "%d ", stats[j] );
		}
		printf("\n");
	}
	return 0;
}

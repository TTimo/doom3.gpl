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
#include "../../../idlib/precompiled.h"
#pragma hdrstop

#include "roqParam.h"

//
// read a parameter file in (true I bloddy well had to do this again) (and yet again to c++)
//

int parseRange(const char *rangeStr,int field, int skipnum[], int startnum[], int endnum[],int numfiles[],bool padding[],int numpadding[] );
int parseTimecodeRange(const char *rangeStr,int field, int skipnum[], int startnum[], int endnum[],int numfiles[],bool padding[],int numpadding[] );

void roqParam::InitFromFile( const char *fileName ) 
{
	idParser *src;
	idToken token;
	int i, readarg;


	src = new idParser( fileName, LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
	if ( !src->IsLoaded() ) {
		delete src;
		common->Printf("Error: can't open param file %s\n", fileName);
		return;
	}
	
	common->Printf("initFromFile: %s\n", fileName);

	fullSearch = false;
	scaleDown = false;
	encodeVideo = false;
	addPath = false;
	screenShots = false;
	startPalette = false;
	endPalette = false;
	fixedPalette = false;
	keyColor = false;
	justDelta = false;
	useTimecodeForRange = false;
	onFrame = 0;
	numInputFiles = 0;
	currentPath[0] = '\0';
	make3DO = false;
	makeVectors = false;
	justDeltaFlag = false;
	noAlphaAtAll = false;
	twentyFourToThirty = false;
	hasSound = false;
	isScaleable = false;
	firstframesize = 56*1024;
	normalframesize = 20000;
	jpegDefault = 85;

	realnum = 0;
	while( 1 ) {
		if ( !src->ReadToken( &token ) ) {
			break;
		}
		
		readarg = 0;
// input dir
		if (token.Icmp( "input_dir") == 0) {
			src->ReadToken( &token );
			addPath = true;
			currentPath = token;
//			common->Printf("  + input directory is %s\n", currentPath );
			readarg++;
			continue;
		}
// input dir
		if (token.Icmp( "scale_down") == 0) {
			scaleDown = true;
//			common->Printf("  + scaling down input\n" );
			readarg++;
			continue;
		}
// full search
		if (token.Icmp( "fullsearch") == 0) {
			normalframesize += normalframesize/2;
			fullSearch = true;
			readarg++;
			continue;
		}
// scaleable
		if (token.Icmp( "scaleable") == 0) {
			isScaleable = true;
			readarg++;
			continue;
		}
// input dir
		if (token.Icmp( "no_alpha") == 0) {
			noAlphaAtAll = true;
//			common->Printf("  + scaling down input\n" );
			readarg++;
			continue;
		}
		if (token.Icmp( "24_fps_in_30_fps_out") == 0) {
			twentyFourToThirty = true;
			readarg++;
			continue;
		}
// video in
		if (token.Icmp( "video_in") == 0) {
			encodeVideo = true;
//			common->Printf("  + Using the video port as input\n");
			continue;
		}
//timecode range
		if (token.Icmp( "timecode") == 0) {
			useTimecodeForRange = true;
			firstframesize = 12*1024;
			normalframesize = 4500;
//			common->Printf("  + Using timecode as range\n");
			continue;
		}
// soundfile for making a .RnR
		if (token.Icmp( "sound") == 0) {
			src->ReadToken( &token );
			soundfile = token;
			hasSound = true;
//			common->Printf("  + Using timecode as range\n");
			continue;
		}
// soundfile for making a .RnR
		if (token.Icmp( "has_sound") == 0) {
			hasSound = true;
			continue;
		}
// outfile	
		if (token.Icmp( "filename") == 0) {
			src->ReadToken( &token );
			outputFilename = token;
			i = strlen(outputFilename);
//			common->Printf("  + output file is %s\n", outputFilename );
			readarg++;
			continue;
		}
// starting palette
		if (token.Icmp( "start_palette") == 0) {
			src->ReadToken( &token );
			sprintf(startPal, "/LocalLibrary/vdxPalettes/%s", token.c_str());
//			common->Error("  + starting palette is %s\n", startPal );
			startPalette = true;
			readarg++;
			continue;
		}
// ending palette
		if (token.Icmp( "end_palette") == 0) {
			src->ReadToken( &token );
			sprintf(endPal, "/LocalLibrary/vdxPalettes/%s", token.c_str());
//			common->Printf("  + ending palette is %s\n", endPal );
			endPalette = true;
			readarg++;
			continue;
		}
// fixed palette
		if (token.Icmp( "fixed_palette") == 0) {
			src->ReadToken( &token );
			sprintf(startPal, "/LocalLibrary/vdxPalettes/%s", token.c_str());
//			common->Printf("  + fixed palette is %s\n", startPal );
			fixedPalette = true;
			readarg++;
			continue;
		}
// these are screen shots
		if (token.Icmp( "screenshot") == 0) {
//			common->Printf("  + shooting screen shots\n" );
			screenShots = true;
			readarg++;
			continue;
		}
//	key_color	r g b	
		if (token.Icmp( "key_color") == 0) {
			keyR = src->ParseInt();
			keyG = src->ParseInt();
			keyB = src->ParseInt();
			keyColor = true;
//			common->Printf("  + key color is %03d %03d %03d\n", keyR, keyG, keyB );
			readarg++;
			continue;
		}
// only want deltas
		if (token.Icmp( "just_delta") == 0) {
//			common->Printf("  + outputting deltas in the night\n" );
//			justDelta = true;
//			justDeltaFlag = true;
			readarg++;
			continue;
		}
// doing 3DO
		if (token.Icmp( "3DO") == 0) {
			make3DO = true;
			readarg++;
			continue;
		}
// makes codebook vector tables
		if (token.Icmp( "codebook") == 0) {
			makeVectors = true;
			readarg++;
			continue;
		}
// set first frame size
		if (token.Icmp( "firstframesize") == 0) {
			firstframesize = src->ParseInt();
			readarg++;
			continue;
		}
// set normal frame size
		if (token.Icmp( "normalframesize") == 0) {
			normalframesize = src->ParseInt();
			readarg++;
			continue;
		}
// set normal frame size
		if (token.Icmp( "stillframequality") == 0) {
			jpegDefault = src->ParseInt();
			readarg++;
			continue;
		}
		if (token.Icmp( "input") == 0) {
			int num_files = 255;

			range = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			padding = (bool *)Mem_ClearedAlloc( num_files * sizeof(bool) );
			padding2 = (bool *)Mem_ClearedAlloc( num_files * sizeof(bool) );
			skipnum = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			skipnum2 = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			startnum = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			startnum2 = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			endnum = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			endnum2 = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			numpadding = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			numpadding2 = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			numfiles = (int *)Mem_ClearedAlloc( num_files * sizeof(int) );
			idStr empty;
			file.AssureSize( num_files, empty );
			file.AssureSize( num_files, empty );

			field = 0;
			realnum = 0;
			do {	
				src->ReadToken(&token);
				if ( token.Icmp( "end_input") != 0 ) {
					idStr arg1, arg2, arg3;

					file[field] = token;
					while (src->ReadTokenOnLine( &token ) && token.Icmp( "[" ) ) {
						file[field].Append( token );
					}

					arg1 = token;
					while (src->ReadTokenOnLine( &token ) && token.Icmp( "[" ) ) {
						arg1 += token;
					}

					arg2 = token;
					while (src->ReadTokenOnLine( &token ) && token.Icmp( "[" ) ) {
						arg2 += token;
					}

					arg3 = token;
					while (src->ReadTokenOnLine( &token ) && token.Icmp( "[" ) ) {
						arg3 += token;
					}

					if ( arg1[0] != '[' ) {
//						common->Printf("  + reading %s\n", file[field] );
						range[field] = 0;
						numfiles[field] = 1;
						realnum++;
					} 
					else {
						if ( arg1[0] == '[' )
						{
							range[field] = 1;
							if (useTimecodeForRange)  {
								realnum += parseTimecodeRange( arg1, field, skipnum, startnum, endnum, numfiles, padding, numpadding);
//								common->Printf("  + reading %s from %d to %d\n", file[field], startnum[field], endnum[field]);
							}
							else {
								realnum += parseRange( arg1, field, skipnum, startnum, endnum, numfiles, padding, numpadding);
//								common->Printf("  + reading %s from %d to %d\n", file[field], startnum[field], endnum[field]);
							}
						}
						else if (( arg1[0] != '[' ) && ( arg2[0] == '[') && ( arg3[0] =='[')) {  //a double ranger...
							int files1,files2;
							
							file2[field] = arg1;
							range[field] = 2;
							files1 = parseRange(arg2, field, skipnum, startnum, endnum, numfiles, padding, numpadding);
//							common->Printf("  + reading %s from %d to %d\n", file[field], startnum[field], endnum[field]);
							files2 = parseRange(arg3, field, skipnum2, startnum2, endnum2, numfiles, padding2, numpadding2);
//							common->Printf("  + reading %s from %d to %d\n", file2[field], startnum2[field], endnum2[field]);
							if (files1 != files2) {
								common->Error( "You had %d files for %s and %d for %s!", files1, arg1.c_str(), files2, arg2.c_str() );
							}
							else	{
								realnum += files1;//not both, they are parallel
							}
						}
						else	{
							common->Error("Error: invalid range on open (%s %s %s)\n", arg1.c_str(), arg2.c_str(), arg3.c_str() );
						}
					} 					
					field++;
				}
			} while (token.Icmp( "end_input"));
		}
	}

	if (TwentyFourToThirty()) realnum = realnum+(realnum>>2);
	numInputFiles = realnum;
	common->Printf("  + reading a total of %d frames in %s\n", numInputFiles, currentPath.c_str() );
	delete src;
}

void roqParam::GetNthInputFileName( idStr &fileName, int n ) {
	int i, myfield, index,hrs,mins,secs,frs;
	char tempfile[33], left[256], right[256], *strp;
	if ( n > realnum ) n = realnum;
// overcome starting at zero by ++ing and then --ing.
	if (TwentyFourToThirty()) { n++; n = (n/5) * 4 + (n % 5); n--; }
	
	i = 0;
	myfield = 0;
	
	while (i <= n) {
		i += numfiles[myfield++];
	}
	myfield--;
	i -= numfiles[myfield];
	
	if ( range[myfield] == 1 ) {
		
		strcpy( left, file[myfield] );
		strp = strstr( left, "*" );
		*strp++ = 0;
		sprintf(right, "%s", strp);
		
		if ( startnum[myfield] <= endnum[myfield] ) {
			index = startnum[myfield] + ((n-i)*skipnum[myfield]);
		} else {
			index = startnum[myfield] - ((n-i)*skipnum[myfield]);
		}
		
		if ( padding[myfield] == true ) {
			if (useTimecodeForRange) {
				hrs = index/(30*60*60) ;
				mins = (index/(30*60)) %60;
				secs = (index/(30)) % 60;
				frs = index % 30;
				sprintf(fileName,"%s%.02d%.02d/%.02d%.02d%.02d%.02d%s",left,hrs,mins,hrs,mins,secs,frs,right);
			}
			else  {
				sprintf(tempfile, "%032d", index );
				sprintf(fileName, "%s%s%s", left, &tempfile[ 32-numpadding[myfield] ], right );
			}
		} else {
			if (useTimecodeForRange) {
				hrs = index/(30*60*60) ;
				mins = (index/(30*60)) %60;
				secs = (index/(30)) % 60;
				frs = index % 30;
				sprintf(fileName,"%s%.02d%.02d/%.02d%.02d%.02d%.02d%s",left,hrs,mins,hrs,mins,secs,frs,right);
			}
			else  {
				sprintf(fileName, "%s%d%s", left, index, right );
			}
		}
	} else if ( range[myfield] == 2 ) {
		
		strcpy( left, file[myfield] );
		strp = strstr( left, "*" );
		*strp++ = 0;
		sprintf(right, "%s", strp);
		
		if ( startnum[myfield] <= endnum[myfield] ) {
			index = startnum[myfield] + ((n-i)*skipnum[myfield]);
		} else {
			index = startnum[myfield] - ((n-i)*skipnum[myfield]);
		}
		
		if ( padding[myfield] == true ) {
			sprintf(tempfile, "%032d", index );
			sprintf(fileName, "%s%s%s", left, &tempfile[ 32-numpadding[myfield] ], right );
		} else {
			sprintf(fileName, "%s%d%s", left, index, right );
		}

		strcpy( left, file2[myfield] );
		strp = strstr( left, "*" );
		*strp++ = 0;
		sprintf(right, "%s", strp);
		
		if ( startnum2[myfield] <= endnum2[myfield] ) {
			index = startnum2[myfield] + ((n-i)*skipnum2[myfield]);
		} else {
			index = startnum2[myfield] - ((n-i)*skipnum2[myfield]);
		}
		
		if ( padding2[myfield] == true ) {
			sprintf(tempfile, "%032d", index );
			fileName += va( "\n%s%s%s", left, &tempfile[ 32-numpadding2[myfield] ], right );
		} else {
			fileName += va( "\n%s%d%s", left, index, right );
		}
	} else {
		fileName = file[myfield];
	}
}

const char* roqParam::GetNextImageFilename( void ) {
	idStr tempBuffer;
	int	i;
	int len;

	GetNthInputFileName( tempBuffer, onFrame++);
	if ( justDeltaFlag == true ) {
		onFrame--;
		justDeltaFlag = false;
	}
	
	if ( addPath == true ) {
		currentFile = currentPath + "/" + tempBuffer;
	} else {
		currentFile = tempBuffer;
   	}
	len = currentFile.Length();
	for(i=0;i<len;i++) {
	    if (currentFile[i] == '^') {
			currentFile[i] = ' ';
	    }
	}
	
	return currentFile.c_str();
}

const char* roqParam::RoqFilename( void ) {
	return outputFilename.c_str();
}

const char* roqParam::SoundFilename( void ) {
	return soundfile.c_str();
}

const char* roqParam::RoqTempFilename( void ) {
	int i, j, len;

	j = 0;
	len = outputFilename.Length();
	for(i=0; i<len; i++ )
		if ( outputFilename[i] == '/' ) j = i;

	sprintf(tempFilename, "/%s.temp", &outputFilename[j+1] );

	return tempFilename.c_str();
}

bool roqParam::Timecode( void ) {
	return useTimecodeForRange;
}

bool roqParam::OutputVectors( void ) {
	return makeVectors;
}

bool roqParam::HasSound( void ) {
	return hasSound;
}

bool roqParam::DeltaFrames( void ) {
	return justDelta;
}

bool roqParam::NoAlpha( void ) {
	return noAlphaAtAll;
}

bool roqParam::SearchType( void ) {
	return fullSearch;
}

bool roqParam::MoreFrames( void ) {
	if (onFrame < numInputFiles) {
		return true;
	} else {
		return false;
	}
}

bool roqParam::TwentyFourToThirty( void ) {
	return twentyFourToThirty;
}

int roqParam::NumberOfFrames( void ) {
	return numInputFiles;
}

int roqParam::FirstFrameSize( void ) {
	return firstframesize;
}

int roqParam::NormalFrameSize( void ) {
	return normalframesize;
}

bool roqParam::IsScaleable( void ) {
	return isScaleable;
}

int roqParam::JpegQuality( void ) {
	return	jpegDefault;
}

int parseRange(const char *rangeStr,int field, int skipnum[], int startnum[], int endnum[],int numfiles[],bool padding[],int numpadding[] ) {
	char start[64], end[64], skip[64];
	char *stptr, *enptr, *skptr;
	int i,realnum;

	i = 1;
	realnum = 0;
	stptr = start;
	enptr = end;
	skptr = skip;
	do {
		*stptr++ = rangeStr[i++];
	} while ( rangeStr[i] >= '0' && rangeStr[i] <= '9' );
	*stptr = '\0';
	if ( rangeStr[i++] != '-' ) {
		common->Error("Error: invalid range on middle \n");
	}
	do {
		*enptr++ = rangeStr[i++];
	} while ( rangeStr[i] >= '0' && rangeStr[i] <= '9' );
	*enptr = '\0';
	if ( rangeStr[i] != ']' ) {
		if ( rangeStr[i++] != '+' ) {
			common->Error("Error: invalid range on close\n");
		}
		do {
			*skptr++ = rangeStr[i++];
		} while ( rangeStr[i] >= '0' && rangeStr[i] <= '9' );
		*skptr = '\0';
		skipnum[field] = atoi( skip );
	} else {
		skipnum[field] = 1;
	}
	startnum[field] = atoi( start );
	endnum[field] = atoi( end );
	numfiles[field] = (abs( startnum[field] - endnum[field] ) / skipnum[field]) + 1;
	realnum += numfiles[field];
	if ( start[0] == '0' && start[1] != '\0' ) {
		padding[field] = true;
		numpadding[field] = strlen( start );
	} else {
		padding[field] = false;
	}
	return realnum;
}

int parseTimecodeRange(const char *rangeStr,int field, int skipnum[], int startnum[], int endnum[],int numfiles[],bool padding[],int numpadding[] )
{
	char start[64], end[64], skip[64];
	char *stptr, *enptr, *skptr;
	int i,realnum,hrs,mins,secs,frs;

	i = 1;//skip the '['
	realnum = 0;
	stptr = start;
	enptr = end;
	skptr = skip;
	do {
		*stptr++ = rangeStr[i++];
	} while ( rangeStr[i] >= '0' && rangeStr[i] <= '9' );
	*stptr = '\0';
	if ( rangeStr[i++] != '-' ) {
		common->Error("Error: invalid range on middle \n");
	}
	do {
		*enptr++ = rangeStr[i++];
	} while ( rangeStr[i] >= '0' && rangeStr[i] <= '9' );
	*enptr = '\0';
	if ( rangeStr[i] != ']' ) {
		if ( rangeStr[i++] != '+' ) {
			common->Error("Error: invalid range on close\n");
		}
		do {
			*skptr++ = rangeStr[i++];
		} while ( rangeStr[i] >= '0' && rangeStr[i] <= '9' );
		*skptr = '\0';
		skipnum[field] = atoi( skip );
	} else {
		skipnum[field] = 1;
	}
	sscanf(start,"%2d%2d%2d%2d",&hrs,&mins,&secs,&frs);
	startnum[field] = hrs*30*60*60 + mins *60*30 + secs*30 +frs;
	sscanf(end,"%2d%2d%2d%2d",&hrs,&mins,&secs,&frs);
	endnum[field] = hrs*30*60*60 + mins *60*30 + secs*30 +frs;
	numfiles[field] = (abs( startnum[field] - endnum[field] ) / skipnum[field]) + 1;
	realnum += numfiles[field];
	if ( start[0] == '0' && start[1] != '\0' ) {
		padding[field] = true;
		numpadding[field] = strlen( start );
	} else {
		padding[field] = false;
	}
	return realnum;
}

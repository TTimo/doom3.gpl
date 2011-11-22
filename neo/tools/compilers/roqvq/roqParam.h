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
#ifndef __roqParam_h__
#define __roqParam_h__

#include "gdefs.h"
#pragma once

class roqParam
{
public:
	
	const char*		RoqFilename( void );
	const char*		RoqTempFilename( void );
	const char*		GetNextImageFilename( void );
	const char*		SoundFilename( void );
	void			InitFromFile( const char *fileName );
	void			GetNthInputFileName( idStr &fileName, int n);
	bool			MoreFrames( void );
	bool			OutputVectors( void );
	bool			Timecode( void );
	bool			DeltaFrames( void );
	bool			NoAlpha( void );
	bool			SearchType( void );
	bool			TwentyFourToThirty( void );
	bool			HasSound( void );
	int				NumberOfFrames( void );
	int				NormalFrameSize( void );
	int				FirstFrameSize( void );
	int				JpegQuality( void );
	bool			IsScaleable( void );

	idStr			outputFilename;
	int				numInputFiles;
private:
	int				*range;
	bool			*padding, *padding2;
	idStrList		file;
	idStrList		file2;
	idStr			soundfile;
	idStr			currentPath;
	idStr			tempFilename;
	idStr			startPal;
	idStr			endPal;
	idStr			currentFile;
	int				*skipnum, *skipnum2;
	int				*startnum, *startnum2;
	int				*endnum, *endnum2;
	int				*numpadding, *numpadding2;
	int				*numfiles;
	byte			keyR, keyG, keyB;
	int				field;
	int				realnum;
	int				onFrame;
	int				firstframesize;
	int				normalframesize;
	int				jpegDefault;

	bool			scaleDown;
	bool			twentyFourToThirty;
	bool			encodeVideo;
	bool			useTimecodeForRange;
	bool			addPath;
	bool			screenShots;
	bool			startPalette;
	bool			endPalette;
	bool			fixedPalette;
	bool			keyColor;
	bool			justDelta;
	bool			make3DO;
	bool			makeVectors;
	bool			justDeltaFlag;
	bool			noAlphaAtAll;
	bool			fullSearch;
	bool			hasSound;
	bool			isScaleable;
};


#endif // roqParam

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
#ifndef __codec_h__
#define __codec_h__

#define MAXERRORMAX 200
#define IPSIZE int
const float MOTION_MIN = 1.0f;
const float MIN_SNR = 3.0f;

#define FULLFRAME	0
#define JUSTMOTION	1

#define VQDATA		double

#include "gdefs.h"
#include "roq.h"
#include "quaddefs.h"

class codec {
public:
	codec();
	~codec();

	void	SparseEncode( void );
	void	EncodeNothing( void );
	void	IRGBtab(void);
	void	InitImages(void);
	void	QuadX( int startX, int startY, int quadSize);
	void	InitQStatus();
	float	Snr(byte *old, byte *bnew, int size);
	void	FvqData( byte *bitmap, int size, int realx, int realy,  quadcel *pquad, bool clamp);
	void	GetData( unsigned char *iData, int qSize, int startX, int startY, NSBitmapImageRep *bitmap);
	int		ComputeMotionBlock( byte *old, byte *bnew, int size);
	void	VqData8( byte *cel, quadcel *pquad);
	void	VqData4( byte *cel, quadcel *pquad);
	void	VqData2( byte *cel, quadcel *pquad);
	int		MotMeanY(void);
	int		MotMeanX(void);
	void	SetPreviousImage( const char*filename, NSBitmapImageRep *timage );
	int		BestCodeword( unsigned char *tempvector, int dimension, VQDATA **codebook );
private:
	
	void	VQ( const int numEntries, const int dimension, const unsigned char *vectors, float *snr, VQDATA **codebook, const bool optimize );
	void	Sort( float *list, int *intIndex, int numElements );
	void	Segment( int *alist, float *flist, int numElements, float rmse);
	void	LowestQuad( quadcel*qtemp, int* status, float* snr, int bweigh);
	void	MakePreviousImage( quadcel *pquad );
	float	GetCurrentRMSE( quadcel *pquad );
	int		GetCurrentQuadOutputSize( quadcel *pquad );
	int		AddQuad( quadcel *pquad, int lownum );

	NSBitmapImageRep	*image;
	NSBitmapImageRep	*newImage;
	NSBitmapImageRep 	*previousImage[2];		// the ones in video ram and offscreen ram
	int					numQuadCels;
	int					whichFrame;
	int					slop;
	bool 				detail;
	int 				onQuad;
	int					initRGBtab;
	quadcel 			*qStatus;
	int					dxMean;
	int					dyMean;
	int					codebooksize;
	int					index2[256];
	int					overAmount;
	int					pixelsWide;
	int					pixelsHigh;
	int					codebookmade;
	bool				used2[256];
	bool				used4[256];
	int					dimension2;
	int					dimension4;

	byte				luty[256];
	byte				*luti;
	VQDATA				**codebook2;
	VQDATA				**codebook4;

};

#endif // __codec_h__

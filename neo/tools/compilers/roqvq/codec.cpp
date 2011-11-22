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

#include "codec.h"

float glimit( const float val ) {
	if (val<0) return 0;
	if (val>255) return 255;
	return val;
}

codec::codec() {
	int i;

	common->Printf("init: initing.....\n");
	codebooksize = 256;
	codebook2 = (VQDATA **) Mem_ClearedAlloc(256*sizeof(VQDATA *));
  	for(i=0; i < 256; i++) {
	    codebook2[i] = (VQDATA *) Mem_ClearedAlloc(16*sizeof(VQDATA));
	}
	codebook4 = (VQDATA **) Mem_ClearedAlloc(256*sizeof(VQDATA *));
  	for(i=0; i < 256; i++) {
	    codebook4[i] = (VQDATA *) Mem_ClearedAlloc(64*sizeof(VQDATA));
	}
	previousImage[0] = 0;
	previousImage[1] = 0;
	image = 0;
	whichFrame = 0;
	qStatus = 0;
	luti = 0;
	overAmount = 0;
	codebookmade = 0;
	slop = 0;
}

codec::~codec()
{
	common->Printf("codec: resetting\n");
	if (qStatus) Mem_Free( qStatus);
	if (luti) Mem_Free(luti);
	if (previousImage[0]) delete previousImage[0];
	if (previousImage[1]) delete previousImage[1];
	qStatus = 0;
	initRGBtab = 0;
	previousImage[0] = 0;
	whichFrame = 0;
	luti = 0;
	return;
}

/* Because Shellsort is a variation on Insertion Sort, it has the same 
 * inconsistency that I noted in the InsertionSort class.  Notice where I 
 * subtract a move to compensate for calling a swap for visual purposes.
 */

void codec::Sort( float *list, int *intIndex, int numElements )
{
#define STRIDE_FACTOR 3 	// good value for stride factor is not well-understood
				// 3 is a fairly good choice (Sedgewick)
    int c,d, stride;
    bool found;

    stride = 1;
    while (stride <= numElements)
      stride = stride*STRIDE_FACTOR +1;
    
    while (stride>(STRIDE_FACTOR-1)) { // loop to sort for each value of stride
    	stride = stride / STRIDE_FACTOR;

	for (c = stride; c < numElements; c++){
	    found = false;
	    d = c-stride;
	    while ((d >= 0) && !found) { // move to left until correct place
	        if (list[d]<list[d+stride]) {
			float ftemp;
			int itemp;
			ftemp = list[d]; list[d] = list[d+stride]; list[d+stride] = ftemp;
			itemp = intIndex[d]; intIndex[d] = intIndex[d+stride]; intIndex[d+stride] = itemp;
		    d -= stride;		// jump by stride factor
		} else
		    found = true;
	    }
	}
    }
}
	
void codec::Segment( int *alist, float *flist, int numElements, float rmse)
{
	int x, y, yy, xx, numc, onf, index, temp, best, a0, a1, a2, a3, bpp, i, len;
	byte	find[16], *lineout, *cbook, *src, *dst;
	float	fy, fcr, fcb;
	idFile *fpcb;
	char cbFile[256], tempcb[256], temptb[256];
	bool doopen;
	float y0,y1,y2,y3,cr,cb;

	doopen = false;
	a0 = a1 = a2 = a3 = 0;

	sprintf( tempcb, "%s.cb", theRoQ->CurrentFilename());
	sprintf( temptb, "%s.tb", theRoQ->CurrentFilename());

	onf = 0;
	len = (int)strlen(tempcb);
	for(x=0;x<len;x++) {
		if (tempcb[x] == '\n') for(y=x;y<len;y++) if (tempcb[y] == '/') x = y+1;
		cbFile[onf++] = tempcb[x];
	}
	cbFile[onf] = 0;

	lineout = (byte *)Mem_ClearedAlloc( 4*1024 );

	common->Printf("trying %s\n", cbFile);
	fpcb = fileSystem->OpenFileRead( cbFile );
	if ( !fpcb ) {
		doopen = true;
		common->Printf("failed....\n");
	} else {
		if ( dimension2 == 16 ) x = 3584; else x = 2560;
		if ( fpcb->Read( lineout, x ) != x ) {
		    doopen = true;
		    common->Printf("failed....\n");
		}
		fileSystem->CloseFile( fpcb );
	}

	if ( doopen ) {
		common->Printf("segment: making %s\n", cbFile);
		numc = numElements;
		if (numElements > numc) numc = numElements;
		onf = 0;

		for(x=0;x<256;x++) {
			for(y=0;y<dimension2;y++) codebook2[x][y] = 0;
			for(y=0;y<dimension4;y++) codebook4[x][y] = 0;
		}

		bpp = image->samplesPerPixel();
		cbook = (byte *)Mem_ClearedAlloc( 3 * image->pixelsWide() * image->pixelsHigh());
		float *snrBook = (float *)Mem_ClearedAlloc( image->pixelsWide() * image->pixelsHigh() );
		dst = cbook;
		int numEntries = 0;
		for (i=0; i<numQuadCels; i++) {
		    if (qStatus[i].size == 8 && qStatus[i].rsnr >= MIN_SNR*4) {
				for(y=qStatus[i].yat;y<qStatus[i].yat+8; y+=4) {
				for(x=qStatus[i].xat;x<qStatus[i].xat+8; x+=4) {
					if (qStatus[i].rsnr == 9999.0f) {
						snrBook[numEntries] = 1.0f;
					} else {
						snrBook[numEntries] = qStatus[i].rsnr;
					}
					numEntries++;
					for(yy=y;yy<(y+4);yy++) {
					for(xx=x;xx<(x+4);xx++) {
						src = image->bitmapData() + (yy*(bpp*image->pixelsWide())) + (xx*bpp);
						memcpy( dst, src, 3); dst += 3;
					}
					}
				}
				}
		    }
		}
		common->Printf("segment: %d 4x4 cels to vq\n", numEntries );

		VQ( numEntries, dimension4, cbook, snrBook, codebook4, true );
		
		dst = cbook;
		numEntries = 0;

		for( i=0; i<256; i++ ) {
			for(y=0;y<4;y+=2) {
			for(x=0;x<4;x+=2) {
				snrBook[numEntries] = 1.0f;
				numEntries++;
				for(yy=y;yy<(y+2);yy++) {
				for(xx=x;xx<(x+2);xx++) {
					dst[0] = codebook4[i][yy*12+xx*3+0];
					dst[1] = codebook4[i][yy*12+xx*3+1];
					dst[2] = codebook4[i][yy*12+xx*3+2];
					dst += 3;
				}
				}
			}
			}
		}
		common->Printf("segment: %d 2x2 cels to vq\n", numEntries);
		
		VQ( numEntries, dimension2, cbook, snrBook, codebook2, false );
		
		Mem_Free(cbook);
		Mem_Free(snrBook);

		index = 0;
		for( onf = 0; onf < 256; onf++ ) {
			numc = 0; fcr = fcb = 0;
			for( x = 0; x < 4; x++ ) {
				fy = RMULT*(float)(codebook2[onf][numc+0]) +
						GMULT*(float)(codebook2[onf][numc+1]) +
							BMULT*(float)(codebook2[onf][numc+2]) + 0.5f;
				 if (fy<0) fy = 0; if (fy>255) fy = 255;

				fcr += RIEMULT*(float)(codebook2[onf][numc+0]);
				fcr += GIEMULT*(float)(codebook2[onf][numc+1]);
				fcr += BIEMULT*(float)(codebook2[onf][numc+2]);

				fcb += RQEMULT*(float)(codebook2[onf][numc+0]);
				fcb += GQEMULT*(float)(codebook2[onf][numc+1]);
				fcb += BQEMULT*(float)(codebook2[onf][numc+2]);

				lineout[index++] = (byte)fy;
				numc += 3;
			}
			fcr = (fcr/4)+128.5f; if (fcr<0) fcr = 0; if (fcr>255) fcr = 255;
			fcb = (fcb/4)+128.5f; if (fcb<0) fcb = 0; if (fcb>255) fcr = 255;
			//common->Printf(" fcr == %f, fcb == %f\n", fcr, fcb );
			lineout[index++] = (byte)fcr;
			lineout[index++] = (byte)fcb;
		}

		for(onf=0;onf<256;onf++) {
			for(y=0;y<4;y+=2) {
			for(x=0;x<4;x+=2) {
				numc = 0;
				for(yy=y;yy<(y+2);yy++) {
					temp = (yy*dimension2)+x*(dimension2/4);
					find[numc++] = (byte)(codebook4[onf][temp+0] + 0.50f);
					find[numc++] = (byte)(codebook4[onf][temp+1] + 0.50f);
					find[numc++] = (byte)(codebook4[onf][temp+2] + 0.50f);
					find[numc++] = (byte)(codebook4[onf][temp+3] + 0.50f);
					find[numc++] = (byte)(codebook4[onf][temp+4] + 0.50f);
					find[numc++] = (byte)(codebook4[onf][temp+5] + 0.50f);
				}
				lineout[index++] = BestCodeword( find, dimension2, codebook2 );
			}
			}
		}

		fpcb = fileSystem->OpenFileWrite( cbFile );
		common->Printf("made up %d entries\n", index);
		fpcb->Write( lineout, index );
		fileSystem->CloseFile( fpcb );
		common->Printf("finished write\n");
	}

	for(y=0;y<256;y++) {
		x = y*6;
		y0 = (float)lineout[x++];
		y1 = (float)lineout[x++];
		y2 = (float)lineout[x++];
		y3 = (float)lineout[x++];
		cb = (float)lineout[x++]; cb -= 128;
		cr = (float)lineout[x  ]; cr -= 128;
		x = 0;
		codebook2[y][x++] = glimit( y0 + 1.40200f*cr );
		codebook2[y][x++] = glimit( y0 - 0.34414f*cb - 0.71414f*cr );
		codebook2[y][x++] = glimit( y0 + 1.77200f*cb );
		codebook2[y][x++] = glimit( y1 + 1.40200f*cr );
		codebook2[y][x++] = glimit( y1 - 0.34414f*cb - 0.71414f*cr );
		codebook2[y][x++] = glimit( y1 + 1.77200f*cb );
		codebook2[y][x++] = glimit( y2 + 1.40200f*cr );
		codebook2[y][x++] = glimit( y2 - 0.34414f*cb - 0.71414f*cr );
		codebook2[y][x++] = glimit( y2 + 1.77200f*cb );
		codebook2[y][x++] = glimit( y3 + 1.40200f*cr );
		codebook2[y][x++] = glimit( y3 - 0.34414f*cb - 0.71414f*cr );
		codebook2[y][x++] = glimit( y3 + 1.77200f*cb );
	}

	index = 6*256;

	for(onf=0;onf<256;onf++) {
		for(y=0;y<4;y+=2) {
		for(x=0;x<4;x+=2) {
			best = lineout[index++];
			numc = 0;
			for(yy=y;yy<(y+2);yy++) {
				temp = (yy*dimension2)+x*(dimension2/4);
				codebook4[onf][temp+0] = codebook2[best][numc++];	//r
				codebook4[onf][temp+1] = codebook2[best][numc++];	//g
				codebook4[onf][temp+2] = codebook2[best][numc++];	//b
				codebook4[onf][temp+3] = codebook2[best][numc++];	//r a
				codebook4[onf][temp+4] = codebook2[best][numc++];	//g r
				codebook4[onf][temp+5] = codebook2[best][numc++];	//b g 
			}
		}
		}
	}

	theRoQ->WriteCodeBook(lineout);
	//PrepareCodeBook();

	Mem_Free(lineout);
}

int	codec::BestCodeword( unsigned char *tempvector, int dimension, VQDATA **codebook )
{
	VQDATA dist;
	VQDATA bestDist = HUGE;
	VQDATA tempvq[64];
	int bestIndex = -1;

	for( int i=0; i<dimension; i++ ) {
		tempvq[i] = tempvector[i];
	}

	for( int i=0; i<256; i++) {
		dist = 0.0;
		for( int x=0; x<dimension; x+=3 ) {
			const VQDATA r0 = codebook[i][x];
			const VQDATA r1 = tempvq[x];
			const VQDATA g0 = codebook[i][x+1];
			const VQDATA g1 = tempvq[x+1];
			const VQDATA b0 = codebook[i][x+2];
			const VQDATA b1 = tempvq[x+2];
			dist += (r0-r1)*(r0-r1);
			if (dist >= bestDist) {
				continue;
			}
			dist += (g0-g1)*(g0-g1);
			if (dist >= bestDist) {
				continue;
			}
			dist += (b0-b1)*(b0-b1);
			if (dist >= bestDist) {
				continue;
			}
		}
		if ( dist < bestDist ) {
			bestDist = dist;
			bestIndex = i;
		}
	}
	return bestIndex;
}    

void codec::SetPreviousImage( const char*filename, NSBitmapImageRep *timage )
{
	if (previousImage[0]) {
		delete previousImage[0];
	}
	if (previousImage[1]) {
		delete previousImage[1];
	}
	common->Printf("setPreviousImage:%s\n", filename);
	
	previousImage[0] = new NSBitmapImageRep( );
	previousImage[1] = new NSBitmapImageRep( );
	whichFrame=1;
	
	*previousImage[0] = *timage;
	*previousImage[1] = *timage;

	pixelsHigh = previousImage[0]->pixelsHigh();
	pixelsWide = previousImage[0]->pixelsWide();

	common->Printf("setPreviousImage: %dx%d\n", pixelsWide, pixelsHigh );
}

void codec::MakePreviousImage( quadcel *pquad )
{
int i, dy, dx, pluck, size, ind, xx, yy, pWide;
int x, y;
byte *rgbmap, *idataA, *fccdictionary;
bool diff;

	for(i=0;i<256;i++) { used2[i] = used4[i] = false; }

	pWide = pixelsWide & 0xfff0;
	if (!previousImage[0]) {
		previousImage[0] = new NSBitmapImageRep( pWide, (pixelsHigh & 0xfff0) );
		previousImage[1] = new NSBitmapImageRep( pWide, (pixelsHigh & 0xfff0) );
	}

	rgbmap = previousImage[(whichFrame&1)]->bitmapData();

	if ((whichFrame&1) == 1) {
		fccdictionary = previousImage[0]->bitmapData(); 
	} else {
		fccdictionary = previousImage[1]->bitmapData();
	}

	idataA = (byte *)Mem_Alloc( 16*16*4 );

	for(i=0;i<numQuadCels;i++) {
		diff = false;
		size = pquad[i].size;
		if (size) {
		switch( pquad[i].status ) {
			case	DEP:
				break;
			case	SLD:
				ind = pquad[i].patten[0];
				used4[ind] = true;
				for( dy=0; dy<size; dy++ ) {
				pluck = (((dy+pquad[i].yat)*pWide)+pquad[i].xat)*4;
				for( dx=0; dx<size; dx++ ) {
					xx = ((dy>>1)*dimension2)+(dx>>1)*(dimension2/4);
					if (rgbmap[pluck+0] != codebook4[ind][xx+0]) diff = true;
					if (rgbmap[pluck+1] != codebook4[ind][xx+1]) diff = true;
					if (rgbmap[pluck+2] != codebook4[ind][xx+2]) diff = true;
					if (dimension4 == 64 && rgbmap[pluck+3] != codebook4[ind][xx+3]) diff = true;
					
					rgbmap[pluck+0] = (byte)codebook4[ind][xx+0];
					rgbmap[pluck+1] = (byte)codebook4[ind][xx+1];
					rgbmap[pluck+2] = (byte)codebook4[ind][xx+2];
					if (dimension4 == 64) 
						rgbmap[pluck+3] = (byte)codebook4[ind][xx+3];
					else
						rgbmap[pluck+3] = 255;
					pluck += 4;
				}
				}
				if (diff == false && whichFrame) common->Printf("drawImage: SLD just changed the same thing\n");
				break;
			case	PAT:
				ind = pquad[i].patten[0];
				used4[ind] = true;
				for( dy=0; dy<size; dy++ ) {
				pluck = (((dy+pquad[i].yat)*pWide)+pquad[i].xat)*4;
				for( dx=0; dx<size; dx++ ) {
					xx = (dy*size*(dimension2/4))+dx*(dimension2/4);
					if (rgbmap[pluck+0] != codebook4[ind][xx+0]) diff = true;
					if (rgbmap[pluck+1] != codebook4[ind][xx+1]) diff = true;
					if (rgbmap[pluck+2] != codebook4[ind][xx+2]) diff = true;
					if (dimension4 == 64 && rgbmap[pluck+3] != codebook4[ind][xx+3]) diff = true;
					
					rgbmap[pluck+0] = (byte)codebook4[ind][xx+0];
					rgbmap[pluck+1] = (byte)codebook4[ind][xx+1];
					rgbmap[pluck+2] = (byte)codebook4[ind][xx+2];
					if (dimension4 == 64) 
						rgbmap[pluck+3] = (byte)codebook4[ind][xx+3];
					else
						rgbmap[pluck+3] = 255;
					pluck += 4;
				}
				}
				if (diff == false && whichFrame) common->Printf("drawImage: PAT just changed the same thing\n");
				break;
			case	CCC:
				dx = 1;
				for(yy=0;yy<4;yy+=2) {
				for(xx=0;xx<4;xx+=2) {
					ind = pquad[i].patten[dx++];
					used2[ind] = true;
					dy = 0;
					for(y=yy;y<(yy+2);y++) {
					for(x=xx;x<(xx+2);x++) {
						pluck = (((y+pquad[i].yat)*pWide)+(pquad[i].xat+x))*4;
						if (rgbmap[pluck+0] != codebook2[ind][dy+0]) diff = true;
						if (rgbmap[pluck+1] != codebook2[ind][dy+1]) diff = true;
						if (rgbmap[pluck+2] != codebook2[ind][dy+2]) diff = true;
						if (dimension4 == 64 && rgbmap[pluck+3] != codebook2[ind][dy+3]) diff = true;
					
						rgbmap[pluck+0] = (byte)codebook2[ind][dy+0];
						rgbmap[pluck+1] = (byte)codebook2[ind][dy+1];
						rgbmap[pluck+2] = (byte)codebook2[ind][dy+2];
						if (dimension4 == 64) {
							rgbmap[pluck+3] = (byte)codebook2[ind][dy+3];
							dy += 4;
						} else {
							rgbmap[pluck+3] = 255;
							dy += 3;
						}
					}
					}
				}
				}
				if (diff == false && whichFrame) {
					/*
					common->Printf("drawImage: CCC just changed the same thing\n");
					common->Printf("sparseEncode: something is wrong here\n");
					common->Printf("xat:    %d\n", pquad[i].xat);
					common->Printf("yat:    %d\n", pquad[i].yat);
					common->Printf("size    %d\n", pquad[i].size);
					common->Printf("type:   %d\n", pquad[i].status);
					common->Printf("motsnr: %0f\n", pquad[i].snr[FCC]);
					common->Printf("cccsnr: %0f\n", pquad[i].snr[CCC]);
					common->Printf("rmse:   %0f\n", pquad[i].rsnr);
					common->Printf("pat0:   %0d\n", pquad[i].patten[1]);
					common->Printf("pat1:   %0d\n", pquad[i].patten[2]);
					common->Printf("pat2:   %0d\n", pquad[i].patten[3]);
					common->Printf("pat3:   %0d\n", pquad[i].patten[4]);
					//exit(1);
					*/
				}
				break;
			case	FCC:
				dx = pquad[i].xat - ((pquad[i].domain >> 8  ) - 128);
				dy = pquad[i].yat - ((pquad[i].domain & 0xff) - 128);
				if (image->pixelsWide()==(image->pixelsHigh()*4)) dx = pquad[i].xat - ((pquad[i].domain >> 8  ) - 128)*2;
				if (theRoQ->Scaleable()) {
					dx = pquad[i].xat - ((pquad[i].domain >> 8  ) - 128)*2;
					dy = pquad[i].yat - ((pquad[i].domain & 0xff) - 128)*2;
				}
//				if (pquad[i].yat == 0) common->Printf("dx = %d, dy = %d, xat = %d\n", dx, dy, pquad[i].xat);
				
				ind = (dy*pWide+dx)*4;
				for( dy=0; dy<size; dy++ ) {
				pluck = (((dy+pquad[i].yat)*pWide)+pquad[i].xat)*4;
				for( dx=0; dx<size; dx++ ) {
					if (rgbmap[pluck+0] != fccdictionary[ind+0]) diff = true;
					if (rgbmap[pluck+1] != fccdictionary[ind+1]) diff = true;
					if (rgbmap[pluck+2] != fccdictionary[ind+2]) diff = true;
					
					rgbmap[pluck+0] = fccdictionary[ind+0];
					rgbmap[pluck+1] = fccdictionary[ind+1];
					rgbmap[pluck+2] = fccdictionary[ind+2];
					rgbmap[pluck+3] = fccdictionary[ind+3];
					pluck += 4; ind += 4;
				}
				ind += (pWide - size)*4;
				}
//				if (diff == false && whichFrame) common->Printf("drawImage: FCC just changed the same thing\n");
				break;
			case	MOT:
				break;
			default:
				common->Error( "bad code!!\n");
				break;
		}
	}
	}
	if (whichFrame == 0) {
			memcpy( previousImage[1]->bitmapData(), previousImage[0]->bitmapData(), pWide*(pixelsHigh & 0xfff0)*4);
	}

	x = 0; y = 0;
	for(i=0;i<256;i++) { 
		if (used4[i]) x++;
		if (used2[i]) y++;
	}

	if (theRoQ->IsQuiet() == false) common->Printf("drawImage: used %d 4x4 and %d 2x2 VQ cels\n", x,y);

	Mem_Free( idataA );
}

void codec::InitImages( void )
{
int x,y, index0, index1, temp;
float ftemp;
byte *lutimage;

	numQuadCels  = ((pixelsWide & 0xfff0)*(pixelsHigh & 0xfff0))/(MINSIZE*MINSIZE);
	numQuadCels += numQuadCels/4 + numQuadCels/16;

	if (qStatus) Mem_Free(qStatus);
	qStatus =  (quadcel *)Mem_ClearedAlloc(numQuadCels*sizeof (quadcel));
	InitQStatus();
//
	if (previousImage[0]) {
		pixelsWide = previousImage[0]->pixelsWide();
		pixelsHigh = previousImage[0]->pixelsHigh();
		temp = ((whichFrame+1)&1);
		if (!luti) luti = (byte *)Mem_Alloc(pixelsWide*pixelsHigh);
		lutimage = previousImage[temp]->bitmapData();
		if (theRoQ->IsQuiet() == false) {
			common->Printf("initImage: remaking lut image using buffer %d\n", temp);
		}
		index0 = index1 = 0;
		for(y=0;y<pixelsHigh; y++) {
		for(x=0;x<pixelsWide; x++) {
			ftemp = RMULT*lutimage[index0+0] + GMULT*lutimage[index0+1] + BMULT*lutimage[index0+2];
			temp = (int)ftemp;
			luti[index1] = temp;
			
			index0 += previousImage[0]->samplesPerPixel();
			index1++;
		}
		}
	}
}


void codec::QuadX( int startX, int startY, int quadSize)
{
int startSize;
int bigx, bigy, lowx, lowy;

	lowx = lowy = 0;
	bigx = pixelsWide & 0xfff0;
	bigy = pixelsHigh & 0xfff0;
	
	if ( (startX >= lowx) && (startX+quadSize) <= (bigx) && (startY+quadSize) <= (bigy) && (startY >= lowy) && quadSize <= MAXSIZE) {
		qStatus[onQuad].size 	= quadSize;
		qStatus[onQuad].xat		= startX;
		qStatus[onQuad].yat		= startY;
		qStatus[onQuad].rsnr	= 999999;
		onQuad++;
	}

	if (quadSize != MINSIZE) {
		startSize = quadSize>>1;
		QuadX( startX           , startY           , startSize );
		QuadX( startX+startSize , startY           , startSize );
		QuadX( startX           , startY+startSize , startSize );
		QuadX( startX+startSize , startY+startSize , startSize );
	}
}

void codec::InitQStatus( void )
{
int i,x,y;

	for (i=0; i<numQuadCels; i++)
		qStatus[i].size = 0;

	onQuad = 0;
	for(y=0;y<pixelsHigh; y+=16) {
	for(x=0;x<pixelsWide; x+=16) {
		QuadX( x, y,16 );
	}
	}
}


void codec::VqData8( byte *cel, quadcel *pquad )
{
byte	tempImage[8*8*4];
int		x, y, i, best, temp;

	i = 0;
	for(y=0;y<4;y++) {
	for(x=0;x<4;x++) {
		temp = y*64+x*8;
		tempImage[i++] = (cel[temp+0]+cel[temp+4]+cel[temp+32]+cel[temp+36])/4;
		tempImage[i++] = (cel[temp+1]+cel[temp+5]+cel[temp+33]+cel[temp+37])/4;
		tempImage[i++] = (cel[temp+2]+cel[temp+6]+cel[temp+34]+cel[temp+38])/4;
		if (dimension4 == 64)
			tempImage[i++] = (cel[temp+3]+cel[temp+7]+cel[temp+35]+cel[temp+39])/4;
	}
	}

	pquad->patten[0] = best = BestCodeword( tempImage, dimension4, codebook4 );

	for(y=0;y<8;y++) {
	for(x=0;x<8;x++) {
		temp = y*32+x*4;
		i = ((y/2)*4*(dimension2/4))+(x/2)*(dimension2/4);
		tempImage[temp+0] = (byte)codebook4[best][i+0];
		tempImage[temp+1] = (byte)codebook4[best][i+1];
		tempImage[temp+2] = (byte)codebook4[best][i+2];
		if (dimension4 == 64)
			tempImage[temp+3] = (byte)codebook4[best][i+3];
		else
			tempImage[temp+3] = 255;
	}
	}

	pquad->snr[SLD] = Snr( cel, tempImage, 8 )+1.0;
}

void codec::VqData4( byte *cel, quadcel *pquad )
{
byte	tempImage[64];
int		i, best, bpp;

//	if (theRoQ->makingVideo] && previousImage[0]) return self;
	if (dimension4 == 64) bpp = 4; else bpp = 3;
	for(i=0;i<16;i++) {
		tempImage[i*bpp+0] = cel[i*4+0];
		tempImage[i*bpp+1] = cel[i*4+1];
		tempImage[i*bpp+2] = cel[i*4+2];
		if (dimension4 == 64) tempImage[i*bpp+3] = cel[i*4+3];
	}

	pquad->patten[0] = best = BestCodeword( tempImage, dimension4, codebook4 );

	for(i=0;i<16;i++) {
		tempImage[i*4+0] = (byte)codebook4[best][i*bpp+0];
		tempImage[i*4+1] = (byte)codebook4[best][i*bpp+1];
		tempImage[i*4+2] = (byte)codebook4[best][i*bpp+2];
		if (dimension4 == 64)
			tempImage[i*4+3] = (byte)codebook4[best][i*bpp+3];
		else
			tempImage[i*4+3] = 255;
	}

	pquad->snr[PAT] = Snr( cel, tempImage, 4 );
}

void codec::VqData2( byte *cel, quadcel *pquad )
{
byte	tempImage[16], tempOut[64];
int		i, j, best,x,y,xx,yy,bpp;

	if (dimension4 == 64) bpp = 4; else bpp = 3;
	j = 1;
	for(yy=0;yy<4;yy+=2) {
	for(xx=0;xx<4;xx+=2) {
		i = 0;
		for(y=yy;y<(yy+2);y++) {
		for(x=xx;x<(xx+2);x++) {
			tempImage[i++] = cel[y*16+x*4+0];
			tempImage[i++] = cel[y*16+x*4+1];
			tempImage[i++] = cel[y*16+x*4+2];
			if (dimension4 == 64) tempImage[i++] = cel[y*16+x*4+3];
		}
		}
		pquad->patten[j++] = best = BestCodeword( tempImage, dimension2, codebook2 );
		i = 0;
		for(y=yy;y<(yy+2);y++) {
		for(x=xx;x<(xx+2);x++) {
			tempOut[y*16+x*4+0] = (byte)codebook2[best][i++];
			tempOut[y*16+x*4+1] = (byte)codebook2[best][i++];
			tempOut[y*16+x*4+2] = (byte)codebook2[best][i++];
			if (dimension4 == 64)
				tempOut[y*16+x*4+3]  = (byte)codebook2[best][i++];
			else
				tempOut[y*16+x*4+3] = 255;
		}
		}
	}
	}

	pquad->snr[CCC] = Snr( cel, tempOut, 4 );
}

void codec::IRGBtab(void)
{
	initRGBtab++;
}

float codec::Snr( byte *old, byte *bnew, int size ) {
int i, j;
float fsnr;
register int ind;

	ind = 0;
	
	for(i=0; i<size; i++) {
	for(j=0; j<size; j++) {
		if (old[3]||bnew[3]) ind += RGBADIST( old, bnew );
		old += 4; bnew += 4;
	}
	}

	fsnr = (float)ind;
	fsnr /= (size*size);
	fsnr = (float)sqrt( fsnr );
	
	return (fsnr);
}

int codec::ComputeMotionBlock( byte *old, byte *bnew, int size )
{
int i,j,snr;

	if (dimension4==64) return 0;	// do not use this for alpha pieces
	snr = 0;

	for(i=0; i<size; i++) {
	for(j=0; j<size; j++) {
		snr += RGBADIST( old, bnew );
		old += 4; bnew += 4;
	}
	}
	snr /= (size*size);
	return ( snr <= MOTION_MIN );
}

void codec::FvqData( byte *bitmap, int size, int realx, int realy,  quadcel *pquad, bool clamp)
{
	int x, y, xLen, yLen, mblur0, ripl, bpp, fabort, temp1;
	int lowX, lowY, onx, ony, sX, sY, depthx, depthy, breakHigh;
	float lowestSNR, fmblur0;
	byte *scale1;
	byte *bitma2;
	int searchY, searchX, xxMean, yyMean;

	if ( !previousImage[0] || dimension4 == 64) {
		return;
	}

	for(x=0; x<(size*size); x++) {
		fmblur0 = RMULT*bitmap[x*4+0] + GMULT*bitmap[x*4+1] + BMULT*bitmap[x*4+2];
		luty[x] = (byte)fmblur0;
	}
	if (!luti) {
		pquad->domain = 0x8080;
		pquad->snr[FCC] = 9999;
		return;
	}

	ony = realy - (realy & 0xfff0);
	onx = realx - (realx & 0xfff0);
	
	xLen = previousImage[0]->pixelsWide();
	yLen = previousImage[0]->pixelsHigh();
	ripl = xLen-size;
	
	breakHigh = 99999999;

	fabort = 0;
	lowX = lowY = -1;
	depthx = depthy = 1;
	searchY = 8;	//16;
	searchX = 8;	//32;
	//if (xLen == (yLen*4)) depthx = 2;
	//if (theRoQ->Scaleable()) depthx = depthy = 2;
	
	if (clamp) { searchX = searchY = 8; }
	searchX = searchX*depthx;
	searchY = searchY*depthy;
	xxMean = dxMean*depthx;
	yyMean = dyMean*depthy;
	
	if (((realx-xxMean)+searchX)<0 ||(((realx-xxMean)-searchX)+depthx+size)>xLen || ((realy-yyMean)+searchY)<0 || (((realy-yyMean)-searchY)+depthy+size)>yLen) {
		pquad->snr[FCC] = 9999;
		return;
	}

	int sPsQ = -1;
	for( sX=(((realx-xxMean)-searchX)+depthx); sX<=((realx-xxMean)+searchX) && !fabort; sX+=depthx ) {
	for( sY=(((realy-yyMean)-searchY)+depthy); sY<=((realy-yyMean)+searchY) && breakHigh; sY+=depthy ) {
	temp1 = xLen*sY+sX;
	if ( sX >= 0 && (sX+size) <= xLen && sY >= 0 && (sY+size) <= yLen ) {
		bpp  = previousImage[0]->samplesPerPixel();
		ripl = (xLen-size)*bpp;
		mblur0 = 0;
		bitma2 = bitmap;
		scale1 = previousImage[((whichFrame+1)&1)]->bitmapData() + temp1*bpp; 
//		mblur0 = 0;
//		bitma2 = luty;
//		scale1 = luti + temp1;
		for( y=0; y<size; y++) {
		    for( x=0; x<size; x++) {
			    mblur0 += RGBADIST( bitma2, scale1);
			    bitma2 += 4; scale1 += 4;
		    }
		    if (mblur0 > breakHigh) {
			break;
		    }
		    scale1 += ripl;
		}
		if (breakHigh > mblur0) {
		    breakHigh = mblur0;
		    lowX = sX;
		    lowY = sY;
		}
	    }
	}
	}
	
	if (lowX != -1 && lowY != -1) {
		bpp  = previousImage[0]->samplesPerPixel();
		ripl = (xLen-size)*bpp;
		mblur0 = 0;
		bitma2 = bitmap;
		scale1 = previousImage[((whichFrame+1)&1)]->bitmapData() + (xLen*lowY+lowX)*bpp; 
		for( y=0; y<size; y++) {
		for( x=0; x<size; x++) {
			mblur0 += RGBADIST( bitma2, scale1 );
			scale1 += 4; bitma2 += 4;
		}
		scale1 += ripl;
		}


		lowestSNR = (float)mblur0;
		lowestSNR /= (size*size);
		lowestSNR = (float)sqrt( lowestSNR );
	
		sX = (realx-lowX+128);
		sY = (realy-lowY+128);

		if (depthx==2) {
			sX = ((realx-lowX)/2+128);
		}
		if (depthy==2) {
			sY = ((realy-lowY)/2+128);
		}
		pquad->domain = (sX<<8)+sY;
		pquad->snr[FCC] = lowestSNR;
	}
}


void codec::GetData( unsigned char *iData,  int qSize, int startX, int startY, NSBitmapImageRep *bitmap)
{
int x,y,yoff,bpp,yend,xend;
byte *iPlane[5];
int r,g,b,a;
	
	yend = qSize+startY;
	xend = qSize+startX;

	if (startY > bitmap->pixelsHigh()) return;

	if (yend > bitmap->pixelsHigh()) yend = bitmap->pixelsHigh();
	if (xend > bitmap->pixelsWide()) xend = bitmap->pixelsWide();

	bpp = bitmap->samplesPerPixel();

	if (bitmap->hasAlpha()) {
		iPlane[0] = bitmap->bitmapData();
		for(y=startY;y<yend;y++) {
			yoff = y*bitmap->pixelsWide()*bpp;
			for(x=startX;x<xend;x++) {
				r = iPlane[0][yoff+(x*bpp)+0];
				g = iPlane[0][yoff+(x*bpp)+1];
				b = iPlane[0][yoff+(x*bpp)+2];
				a = iPlane[0][yoff+(x*bpp)+3];
				*iData++ = r; *iData++ = g; *iData++ = b; *iData++ = a;
			}
		}
	} else {
		iPlane[0] = bitmap->bitmapData();
		for(y=startY;y<yend;y++) {
			yoff = y*bitmap->pixelsWide()*bpp;
			for(x=startX;x<xend;x++) {
				r = iPlane[0][yoff+(x*bpp)+0];
				g = iPlane[0][yoff+(x*bpp)+1];
				b = iPlane[0][yoff+(x*bpp)+2];
				*iData++ = r; *iData++ = g; *iData++ = b;
				*iData++ = 255;
			}
		}
	}
}

void codec::LowestQuad( quadcel*qtemp, int* status, float* snr, int bweigh)
{
float wtemp;
float quickadd[DEAD];
int i;

	quickadd[CCC] = 1;
	quickadd[SLD] = 1;
	quickadd[MOT] = 1;
	quickadd[FCC] = 1;
	quickadd[PAT] = 1;
/*
	if (slop > theRoQ->NormalFrameSize()) {
		quickadd[CCC] = 0.5f;
		quickadd[PAT] = 1.0f;
	}
*/
	wtemp = 99999;

	for(i=(DEAD-1);i>0;i--) {
		if ( qtemp->snr[i]*quickadd[i] < wtemp ) {
			*status = i;
			*snr 	= qtemp->snr[i];
			wtemp 	= qtemp->snr[i]*quickadd[i];
		}
	}

	if ( qtemp->mark ) *status = MOT;
}

int	codec::GetCurrentQuadOutputSize( quadcel *pquad )
{
int	totalbits, i, totalbytes;
int quickadd[DEAD+1];
	
	totalbits = 0;

	quickadd[DEP] = 2;
	quickadd[SLD] = 10;
	quickadd[PAT] = 10;
	quickadd[CCC] = 34;
	quickadd[MOT] = 2;
	quickadd[FCC] = 10;
	quickadd[DEAD] = 0;

	for( i=0; i<numQuadCels; i++ ) {
		if (pquad[i].size && pquad[i].size < 16 ) totalbits += quickadd[pquad[i].status];
	}

	totalbytes = (totalbits >> 3)+2;
	return (totalbytes);
}

float codec::GetCurrentRMSE( quadcel *pquad )
{
int	i, j;
double totalbits;

	totalbits = 0;
	j = 0;
	for( i=0; i<numQuadCels; i++ ) {
		if (pquad[i].size && pquad[i].status && pquad[i].status != DEAD) {
			if (pquad[i].size ==  8) { totalbits += pquad[i].rsnr*4;  j += 4; }
			if (pquad[i].size ==  4) { totalbits += pquad[i].rsnr*1;  j += 1; }
		}
	}
	totalbits /= j;
	return ((float)totalbits);
}

int codec::AddQuad( quadcel *pquad, int lownum )
{
int i, nx, nsize;
float newsnr, cmul;
byte *idataA, *idataB;

	if (lownum != -1) {
		
		if (pquad[lownum].size == 8) {
			nx = 1; nsize = 4; cmul = 1;
		} else {
			nx = 5; nsize = 8; cmul = 4;
		}	
		newsnr = 0;
		idataA = (byte *)Mem_Alloc(8*8*4);
		idataB = (byte *)Mem_Alloc(8*8*4);
		for( i=lownum+1; i<lownum+(nx*4)+1; i+=nx ) {
			pquad[i].size = nsize;
			GetData( idataA, pquad[i].size, pquad[i].xat, pquad[i].yat, image );
			VqData4( idataA ,&pquad[i] );
			VqData2( idataA, &pquad[i] );
			if (previousImage[0]) {
				FvqData( idataA, pquad[i].size, pquad[i].xat, pquad[i].yat, &pquad[i], true );
				GetData( idataB, pquad[i].size, pquad[i].xat, pquad[i].yat, previousImage[whichFrame&1] );
				pquad[i].snr[MOT] = Snr( idataA, idataB, pquad[i].size );
				if (ComputeMotionBlock( idataA, idataB, pquad[i].size ) && !theRoQ->IsLastFrame() && !detail) {
					pquad[i].mark = true;
				}
			}
			LowestQuad( &pquad[i] ,&pquad[i].status, &pquad[i].rsnr, true );
			newsnr += pquad[i].rsnr;
		}
		Mem_Free(idataA); Mem_Free(idataB);
		newsnr /= 4;
		LowestQuad( &pquad[lownum], &pquad[lownum].status, &pquad[lownum].rsnr, false );
		
		if (   pquad[lownum+nx*0+1].status == MOT && pquad[lownum+nx*1+1].status == MOT
			&& pquad[lownum+nx*2+1].status == MOT && pquad[lownum+nx*3+1].status == MOT  
			&& nsize == 4) { newsnr = 9999;	pquad[lownum].status = MOT; }
				
		if ( pquad[lownum].rsnr > newsnr ) {
			pquad[lownum].status = DEP;
			pquad[lownum].rsnr   = 0;
			for( i=lownum+1; i<lownum+(nx*4)+1; i+=nx ) {
				theRoQ->MarkQuadx( pquad[i].xat, pquad[i].yat, nsize, pquad[i].rsnr, qStatus[i].status );
			}
		} else {
			theRoQ->MarkQuadx( pquad[lownum].xat, pquad[lownum].yat, nsize*2, pquad[lownum].rsnr, qStatus[lownum].status );
			pquad[lownum+nx*0+1].status = 0;			
			pquad[lownum+nx*1+1].status = 0;			
			pquad[lownum+nx*2+1].status = 0;			
			pquad[lownum+nx*3+1].status = 0;			
			pquad[lownum+nx*0+1].size = 0;			
			pquad[lownum+nx*1+1].size = 0;			
			pquad[lownum+nx*2+1].size = 0;			
			pquad[lownum+nx*3+1].size = 0;			
		}
	} else {
		lownum = -1;
	}
	return lownum;
}

int codec::MotMeanX( void )
{
	return dxMean;
}

int codec::MotMeanY( void )
{
	return dyMean;
}

void codec::SparseEncode( void )
{
int i, j, osize, fsize, num[DEAD+1], *ilist, onf, ong, wtype, temp;
float *flist, sRMSE, numredo;
byte *idataA, *idataB;

	osize = 8;

	image = theRoQ->CurrentImage();
	newImage = 0;

	pixelsHigh = image->pixelsHigh();
	pixelsWide = image->pixelsWide();

	dimension2 = 12; dimension4 = 48;
	if (image->hasAlpha()&&(theRoQ->ParamNoAlpha() == false)) { dimension2 = 16; dimension4 = 64; }

	idataA = (byte *)Mem_Alloc( 16*16*4 );
	idataB = (byte *)Mem_Alloc( 16*16*4 );

	if (!previousImage[0]) common->Printf("sparseEncode: sparsely encoding a %d,%d image\n", pixelsWide, pixelsHigh);
	InitImages();

	flist = (float *)Mem_ClearedAlloc( (numQuadCels+1) *sizeof(float) );
	ilist = (int *)Mem_ClearedAlloc( (numQuadCels+1) *sizeof(int  ) );


	fsize = 56*1024;
	if (theRoQ->NumberOfFrames()>2) {
		if (previousImage[0]) fsize = theRoQ->NormalFrameSize(); else fsize = theRoQ->FirstFrameSize();
		if (theRoQ->HasSound() && fsize > 6000 && previousImage[0]) fsize = 6000;
	}
	fsize += (slop/50);
	if (fsize > 64000) {
	    fsize = 64000;
	}
	if (previousImage[0] && fsize > theRoQ->NormalFrameSize()*2) {
	    fsize = theRoQ->NormalFrameSize()*2;
	}
	dxMean = dyMean = 0;
	if (previousImage[0]) wtype = 1; else wtype = 0;

	for( i=0; i<numQuadCels; i++ ) {
		for(j=0;j<DEAD;j++) qStatus[i].snr[j] = 9999;
		qStatus[i].mark = false;
		if ( qStatus[i].size == osize ) {
			if (previousImage[0]) {
				GetData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, image );
				GetData( idataB, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, previousImage[whichFrame&1] );
				qStatus[i].snr[MOT] = Snr( idataA, idataB, qStatus[i].size );
				if (ComputeMotionBlock( idataA, idataB, qStatus[i].size ) && !theRoQ->IsLastFrame()) {
					qStatus[i].mark = true;
				}
				if (!qStatus[i].mark) {
					FvqData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, &qStatus[i], false );
				}
			}
			LowestQuad( &qStatus[i], &qStatus[i].status, &qStatus[i].rsnr, wtype );
			if (qStatus[i].rsnr < 9999)
				theRoQ->MarkQuadx( qStatus[i].xat, qStatus[i].yat, qStatus[i].size, qStatus[i].rsnr, qStatus[i].status );
		} else {
			if ( qStatus[i].size < osize ) {
				qStatus[i].status = 0;
				qStatus[i].size   = 0;
			} else {
				qStatus[i].status = DEP;
				qStatus[i].rsnr   = 0;
			}
		}
	}
//
// the quad is complete, so status can now be used for quad decomposition
// the first thing to do is to set it up for all the 4x4 cels to get output
// and then recurse from there to see what's what
//
	sRMSE = GetCurrentRMSE( qStatus );

	if (theRoQ->IsQuiet() == false) {
		common->Printf("sparseEncode: rmse of quad0 is %f, size is %d (meant to be %d)\n", sRMSE, GetCurrentQuadOutputSize(qStatus), fsize );
	}
		
	onf = 0;
	for(i=0;i<numQuadCels;i++) {
		if ( qStatus[i].size && qStatus[i].status != DEP) {
			flist[onf] = qStatus[i].rsnr;
			ilist[onf] = i;
			onf++;
		}
	}

	Sort( flist, ilist, onf );
	Segment( ilist, flist, onf, GetCurrentRMSE( qStatus ));

	temp = dxMean = dyMean = 0;
	/*
	for( i=0; i<numQuadCels; i++ ) {
		if (qStatus[i].size && qStatus[i].status == FCC) {
			dxMean += (qStatus[i].domain >> 8  ) - 128;
			dyMean += (qStatus[i].domain & 0xff) - 128;
			temp++;
		}
	}
	if (temp) { dxMean /= temp; dyMean /= temp; }	
	*/
	common->Printf("sparseEncode: dx/dy mean is %d,%d\n", dxMean, dyMean);

	numredo = 0;
	detail = false;
	if (codebookmade && whichFrame>4) fsize -= 256;
	temp = 0;
	for( i=0; i<numQuadCels; i++ ) {
		if ( qStatus[i].size == osize && qStatus[i].mark == false && qStatus[i].snr[MOT] > 0 ) {
			GetData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, image );
			if (osize == 8) VqData8( idataA, &qStatus[i] );
			if (previousImage[0]) {
				int dx,dy;
				dx = (qStatus[i].domain >> 8  ) - 128 - dxMean + 8;
				dy = (qStatus[i].domain & 0xff) - 128 - dyMean + 8;
 				if (dx<0||dx>15||dy<0||dy>15) {
					qStatus[i].snr[FCC] = 9999;
					temp++;
					FvqData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, &qStatus[i], true );
					dx = (qStatus[i].domain >> 8  ) - 128 - dxMean + 8;
					dy = (qStatus[i].domain & 0xff) - 128 - dyMean + 8;
 					if ((dx<0||dx>15||dy<0||dy>15)&&qStatus[i].snr[FCC]!=9999&&qStatus[i].status==FCC) {
						common->Printf("sparseEncode: something is wrong here, dx/dy is %d,%d after being clamped\n", dx, dy);
						common->Printf("xat:    %d\n", qStatus[i].xat);
						common->Printf("yat:    %d\n", qStatus[i].yat);
						common->Printf("size    %d\n", qStatus[i].size);
						common->Printf("type:   %d\n", qStatus[i].status);
						common->Printf("mot:    %04x\n", qStatus[i].domain);
						common->Printf("motsnr: %0f\n", qStatus[i].snr[FCC]);
						common->Printf("rmse:   %0f\n", qStatus[i].rsnr);
						common->Error("need to go away now\n");
					}
				}
			}
			LowestQuad( &qStatus[i], &qStatus[i].status, &qStatus[i].rsnr, wtype );
			theRoQ->MarkQuadx( qStatus[i].xat, qStatus[i].yat, qStatus[i].size, qStatus[i].rsnr, qStatus[i].status );
			/*
			if (qStatus[i].status==FCC && qStatus[i].snr[FCC]>qStatus[i].snr[SLD]) {
						common->Printf("sparseEncode: something is wrong here\n");
						common->Printf("xat:    %d\n", qStatus[i].xat);
						common->Printf("yat:    %d\n", qStatus[i].yat);
						common->Printf("size    %d\n", qStatus[i].size);
						common->Printf("type:   %d\n", qStatus[i].status);
						common->Printf("mot:    %04x\n", qStatus[i].domain);
						common->Printf("motsnr: %0f\n", qStatus[i].snr[FCC]);
						common->Printf("sldsnr: %0f\n", qStatus[i].snr[SLD]);
						common->Printf("rmse:   %0f\n", qStatus[i].rsnr);
						//common->Error("need to go away now\n");
			}
			*/
		}
	}

	if (theRoQ->IsQuiet() == false) {
		common->Printf("sparseEncode: rmse of quad0 is %f, size is %d (meant to be %d)\n", GetCurrentRMSE( qStatus ), GetCurrentQuadOutputSize( qStatus ), fsize );
		common->Printf("sparseEncode: %d outside fcc limits\n", temp);
	}
		
	onf = 0;
	for(i=0;i<numQuadCels;i++) {
		if ( qStatus[i].size && qStatus[i].status != DEP) {
			flist[onf] = qStatus[i].rsnr;
			ilist[onf] = i;
			onf++;
		}
	}
	
	Sort(flist, ilist, onf );

	ong = 0; detail = false;
	
	while ( GetCurrentQuadOutputSize(qStatus) < fsize && ong < onf && flist[ong] > 0 && qStatus[ilist[ong]].mark == false) {
//		badsnr = [self getCurrentRMSE: qStatus]; 
		osize = AddQuad( qStatus, ilist[ong++] );
//		if ([self getCurrentRMSE: qStatus] >= badsnr) {
//		    break;
//		}
	}

	if ( GetCurrentQuadOutputSize( qStatus ) < fsize) {
		ong = 0;
		while ( GetCurrentQuadOutputSize(qStatus) < fsize && ong < onf) {
//			badsnr = [self getCurrentRMSE: qStatus]; 
			i = ilist[ong++];
			if (qStatus[i].mark) {
				detail = false; 
				qStatus[i].mark = false;
				GetData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, image );
				if (qStatus[i].size == 8) VqData8( idataA, &qStatus[i] );
				if (qStatus[i].size == 4) VqData4( idataA, &qStatus[i] );
				if (qStatus[i].size == 4) VqData2( idataA, &qStatus[i] );
				if (previousImage[0]) {
					FvqData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, &qStatus[i], true );
				}
				LowestQuad( &qStatus[i], &qStatus[i].status, &qStatus[i].rsnr, wtype );
				if (qStatus[i].rsnr <= MIN_SNR) {
				    break;
				}
				theRoQ->MarkQuadx( qStatus[i].xat, qStatus[i].yat, qStatus[i].size, qStatus[i].rsnr, qStatus[i].status );
			}
//			if ([self getCurrentRMSE: qStatus] >= badsnr) {
//			    break;
//			}
		}
		ong = 0;
		while ( GetCurrentQuadOutputSize( qStatus ) < fsize && ong < onf && flist[ong] > 0) {
//			badsnr = [self getCurrentRMSE: qStatus]; 
			i = ilist[ong++];
//			if (qStatus[i].rsnr <= MIN_SNR) {
//			    break;
//			}
			detail = true;
			osize = AddQuad( qStatus, i  );
//			if ([self getCurrentRMSE: qStatus] >= badsnr) {
//			    break;
//			}
		}
	}

	common->Printf("sparseEncode: rmse of frame %d is %f, size is %d\n", whichFrame, GetCurrentRMSE(qStatus), GetCurrentQuadOutputSize(qStatus) );

	if (previousImage[0])
	    fsize = theRoQ->NormalFrameSize();
	else
	    fsize = theRoQ->FirstFrameSize();
	    
	slop += (fsize - GetCurrentQuadOutputSize(qStatus));
	
	if (theRoQ->IsQuiet() == false) {
		for(i=0;i<DEAD;i++) num[i] = 0;
		j = 0;
		for( i=0; i<numQuadCels; i++ ) {
			if (qStatus[i].size == 8 && qStatus[i].status) {
				if (qStatus[i].status < DEAD) num[qStatus[i].status]++; j++;
			}
		}
		common->Printf("sparseEncode: for 08x08 CCC = %d, FCC = %d, MOT = %d, SLD = %d, PAT = %d\n", num[CCC], num[FCC], num[MOT], num[SLD], num[PAT]);

		for(i=0;i<DEAD;i++) num[i] = 0;
		for( i=0; i<numQuadCels; i++ ) {
			if (qStatus[i].size == 4 && qStatus[i].status) {
				if (qStatus[i].status < DEAD) num[qStatus[i].status]++; j++;
			}
		}
		common->Printf("sparseEncode: for 04x04 CCC = %d, FCC = %d, MOT = %d, SLD = %d, PAT = %d\n", num[CCC], num[FCC], num[MOT], num[SLD], num[PAT]);

		common->Printf("sparseEncode: average RMSE = %f, numActiveQuadCels = %d, estSize = %d, slop = %d \n", GetCurrentRMSE(qStatus), j, GetCurrentQuadOutputSize(qStatus), slop);
	}

	theRoQ->WriteFrame( qStatus );
	MakePreviousImage( qStatus );

	Mem_Free(idataA);
	Mem_Free(idataB);
	Mem_Free(flist);
	Mem_Free(ilist);
	if (newImage) delete newImage;

	whichFrame++;
}

void codec::EncodeNothing( void )
{
int i, j, osize, fsize, num[DEAD+1], *ilist, wtype;
float *flist, sRMSE;
byte *idataA, *idataB;

	osize = 8;

	image = theRoQ->CurrentImage();
	newImage = 0;

	pixelsHigh = image->pixelsHigh();
	pixelsWide = image->pixelsWide();

	dimension2 = 12; dimension4 = 48;
	if (image->hasAlpha()&&(theRoQ->ParamNoAlpha() == false)) { dimension2 = 16; dimension4 = 64; }

	idataA = (byte *)Mem_Alloc( 16*16*4 );
	idataB = (byte *)Mem_Alloc( 16*16*4 );

	if (!previousImage[0]) common->Printf("sparseEncode: sparsely encoding a %d,%d image\n", pixelsWide, pixelsHigh);
	InitImages();

	flist = (float *)Mem_ClearedAlloc( (numQuadCels+1) * sizeof(float) );
	ilist = (int *)Mem_ClearedAlloc( (numQuadCels+1) * sizeof(int  ) );


	fsize = 56*1024;
	if (theRoQ->NumberOfFrames()>2) {
		if (previousImage[0]) fsize = theRoQ->NormalFrameSize(); else fsize = theRoQ->FirstFrameSize();
		if (theRoQ->HasSound() && fsize > 6000 && previousImage[0]) fsize = 6000;
	}
	
	dxMean = dyMean = 0;
	if (previousImage[0]) wtype = 1; else wtype = 0;

	for( i=0; i<numQuadCels; i++ ) {
		for(j=0;j<DEAD;j++) qStatus[i].snr[j] = 9999;
		qStatus[i].mark = false;
		if ( qStatus[i].size == osize ) {
			if (previousImage[0]) {
				GetData( idataA, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, image );
				GetData( idataB, qStatus[i].size, qStatus[i].xat, qStatus[i].yat, previousImage[whichFrame&1] );
				qStatus[i].snr[MOT] = Snr( idataA, idataB, qStatus[i].size );
			}
			LowestQuad( &qStatus[i], &qStatus[i].status, &qStatus[i].rsnr, wtype );
			if (qStatus[i].rsnr < 9999)
				theRoQ->MarkQuadx( qStatus[i].xat, qStatus[i].yat, qStatus[i].size, qStatus[i].rsnr, qStatus[i].status );
		} else {
			if ( qStatus[i].size < osize ) {
				qStatus[i].status = 0;
				qStatus[i].size   = 0;
			} else {
				qStatus[i].status = DEP;
				qStatus[i].rsnr   = 0;
			}
		}
	}
//
// the quad is complete, so status can now be used for quad decomposition
// the first thing to do is to set it up for all the 4x4 cels to get output
// and then recurse from there to see what's what
//
	sRMSE = GetCurrentRMSE( qStatus );

	common->Printf("sparseEncode: rmse of frame %d is %f, size is %d\n", whichFrame, sRMSE, GetCurrentQuadOutputSize( qStatus ) );

	
	if (theRoQ->IsQuiet() == false) {
		for(i=0;i<DEAD;i++) num[i] = 0;
		j = 0;
		for( i=0; i<numQuadCels; i++ ) {
			if (qStatus[i].size == 8 && qStatus[i].status) {
				if (qStatus[i].status < DEAD) num[qStatus[i].status]++; j++;
			}
		}
		common->Printf("sparseEncode: for 08x08 CCC = %d, FCC = %d, MOT = %d, SLD = %d, PAT = %d\n", num[CCC], num[FCC], num[MOT], num[SLD], num[PAT]);

		for(i=0;i<DEAD;i++) num[i] = 0;
		for( i=0; i<numQuadCels; i++ ) {
			if (qStatus[i].size == 4 && qStatus[i].status) {
				if (qStatus[i].status < DEAD) num[qStatus[i].status]++; j++;
			}
		}
		common->Printf("sparseEncode: for 04x04 CCC = %d, FCC = %d, MOT = %d, SLD = %d, PAT = %d\n", num[CCC], num[FCC], num[MOT], num[SLD], num[PAT]);

		common->Printf("sparseEncode: average RMSE = %f, numActiveQuadCels = %d, estSize = %d \n", GetCurrentRMSE(qStatus), j, GetCurrentQuadOutputSize(qStatus));
	}

	theRoQ->WriteFrame( qStatus );
	MakePreviousImage( qStatus );

	Mem_Free(idataA);
	Mem_Free(idataB);
	Mem_Free(flist);
	Mem_Free(ilist);
	if (newImage) delete newImage;

	whichFrame++;
}

void codec::VQ( const int numEntries, const int dimension, const unsigned char *vectors, float *import, VQDATA **codebook, const bool optimize ) {
	int		startMsec = Sys_Milliseconds();

	if (numEntries <= 256) {
		//
		// copy the entries into the codebooks
		//
		for( int i=0; i<numEntries; i++ ) {
			for( int j=0; j<dimension; j++ ) {
				codebook[i][j] = vectors[j+i*dimension];	
			}
		}
		return;
	}
	//
	// okay, we need to wittle this down to less than 256 entries
	//

	// get rid of identical entries

	int i, j, x, ibase, jbase;

	bool *inuse = (bool *)_alloca( numEntries * sizeof(bool) );
	float *snrs = (float *)_alloca( numEntries * sizeof(float) );
	int *indexes = (int *)_alloca( numEntries * sizeof(int) );
	int *indexet = (int *)_alloca( numEntries * sizeof(int) );

	int numFinalEntries = numEntries;
	for( i=0; i<numEntries; i++ ) {
		inuse[i] = true;
		snrs[i] = -1.0f;
		indexes[i] = -1;
		indexet[i] = -1;
	}
	
	for( i=0; i<numEntries-1; i++ ) {
		for( j=i+1; j<numEntries; j++ ) {
			if (inuse[i] && inuse[j]) {
				if (!memcmp( &vectors[i*dimension], &vectors[j*dimension], dimension)) {
					inuse[j] = false;
					numFinalEntries--;
					import[i] += import[j];
				}
			}
		}
	}

	common->Printf("VQ: has %d entries to process\n", numFinalEntries );

	//
	// are we done?
	//
	int end;
	if (numFinalEntries > 256) {
		//
		// find the closest two and eliminate one
		//
		double bestDist = HUGE;
		double dist, simport;
		int bestIndex = -1;
		int bestOtherIndex = 0;
		int aentries = 0;
		for( i=0; i<numEntries-1; i++ ) {
			if (inuse[i]) {
				end = numEntries;
				if (optimize) {
					if (numFinalEntries>8192) {
						end = i+32;
					} else if (numFinalEntries>4096) {
						end = i+64;
					} else if (numFinalEntries>2048) {
						end = i+128;
					} else if (numFinalEntries>1024) {
						end = i+256;
					} else if (numFinalEntries>512) {
						end = i+512;
					}
					if (end>numEntries) {
						end = numEntries;
					}
				}
				ibase = i*dimension;
				for( j=i+1; j<end; j++ ) {
					if (inuse[j]) {
						dist = 0.0;
						jbase = j*dimension;
						for( x=0; x<dimension; x+=3 ) {
#if 0
							r0 = (float)vectors[ibase+x];
							r1 = (float)vectors[jbase+x];
							g0 = (float)vectors[ibase+x+1];
							g1 = (float)vectors[jbase+x+1];
							b0 = (float)vectors[ibase+x+2];
							b1 = (float)vectors[jbase+x+2];
							dist += idMath::Sqrt16( (r0-r1)*(r0-r1) + (g0-g1)*(g0-g1) + (b0-b1)*(b0-b1) );
#else
							// JDC: optimization
							int	dr = vectors[ibase+x] - vectors[jbase+x];
							int	dg = vectors[ibase+x+1] - vectors[jbase+x+1];
							int	db = vectors[ibase+x+2] - vectors[jbase+x+2];
							dist += idMath::Sqrt16( dr * dr + dg * dg + db * db );
#endif
						}
						simport = import[i] * import[j];
						dist *= simport;
						if ( dist < bestDist ) {
							bestDist = dist;
							bestIndex = i;
							bestOtherIndex = j;
						}
					}
				}
				snrs[aentries] = bestDist;
				indexes[aentries] = bestIndex;
				indexet[aentries] = bestOtherIndex;
				aentries++;
			}
		}

		//
		// until we have reduced it to 256 entries, find one to toss
		//
		do {
			bestDist = HUGE;
			bestIndex = -1;
			bestOtherIndex = -1;
			if (optimize) {
				for( i=0; i<aentries; i++ ) {
					if (inuse[indexes[i]] && inuse[indexet[i]] ) {
						if ( snrs[i] < bestDist ) {
							bestDist = snrs[i];
							bestIndex = indexes[i];
							bestOtherIndex = indexet[i];
						}
					}
				}
			}
			if (bestIndex == -1 || !optimize) {
				bestDist = HUGE;
				bestIndex = -1;
				bestOtherIndex = 0;
				aentries = 0;
				for( i=0; i<numEntries-1; i++ ) {
					if (!inuse[i]) {
						continue;
					}
					end = numEntries;
					if (optimize) {
						if (numFinalEntries>8192) {
							end = i+32;
						} else if (numFinalEntries>4096) {
							end = i+64;
						} else if (numFinalEntries>2048) {
							end = i+128;
						} else if (numFinalEntries>1024) {
							end = i+256;
						} else if (numFinalEntries>512) {
							end = i+512;
						}
					}
					if (end>numEntries) {
						end = numEntries;
					}
					ibase = i*dimension;
					for( j=i+1; j<end; j++ ) {
						if ( !inuse[j]) {
							continue;
						}
						dist = 0.0;
						jbase = j*dimension;
						simport = import[i] * import[j];
						float	scaledBestDist = bestDist / simport;
						for( x=0; x<dimension; x+=3 ) {
#if 0
							r0 = (float)vectors[ibase+x];
							r1 = (float)vectors[jbase+x];
							g0 = (float)vectors[ibase+x+1];
							g1 = (float)vectors[jbase+x+1];
							b0 = (float)vectors[ibase+x+2];
							b1 = (float)vectors[jbase+x+2];
							dist += idMath::Sqrt16( (r0-r1)*(r0-r1) + (g0-g1)*(g0-g1) + (b0-b1)*(b0-b1) );
#else
							// JDC: optimization
							int	dr = vectors[ibase+x] - vectors[jbase+x];
							int	dg = vectors[ibase+x+1] - vectors[jbase+x+1];
							int	db = vectors[ibase+x+2] - vectors[jbase+x+2];
							dist += idMath::Sqrt16( dr * dr + dg * dg + db * db );
							if ( dist > scaledBestDist ) {
								break;
							}
#endif
						}
						dist *= simport;
						if ( dist < bestDist ) {
							bestDist = dist;
							bestIndex = i;
							bestOtherIndex = j;
						}
					}
					snrs[aentries] = bestDist;
					indexes[aentries] = bestIndex;
					indexet[aentries] = bestOtherIndex;
					aentries++;
				}
			}
			//
			// and lose one
			//
			inuse[bestIndex] = false;
			numFinalEntries--;
			import[bestOtherIndex] += import[bestIndex];
			if ((numFinalEntries&511)==0) {
				common->Printf("VQ: has %d entries to process\n", numFinalEntries );
				session->UpdateScreen();
			}
		} while (numFinalEntries > 256);
	}
	//
	// copy the entries into the codebooks
	//
	int onEntry = 0;
	for( i=0; i<numEntries; i++ ) {
		if (inuse[i]) {
			ibase = i*dimension;
			for( x=0; x<dimension; x++ ) {
				codebook[onEntry][x] = vectors[ibase + x];	
			}
			if (onEntry == 0) {
				common->Printf("First vq = %d\n ", i);
			}
			if (onEntry == 255) {
				common->Printf("last vq = %d\n", i);
			}
			onEntry++;
		}
	}

	int		endMsec = Sys_Milliseconds();
	common->Printf( "VQ took %i msec\n", endMsec - startMsec );
}


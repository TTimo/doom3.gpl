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

#ifdef ID_DEBUG_MEMORY
#undef new
#endif

#include "math.h"
#include "CDIB.h"

// Original ColorPicker/DIB source by Rajiv Ramachandran <rrajivram@hotmail.com>
// included with Permission from the author

#define BIG_DISTANCE 10000000L

#define DIST(r1,g1,b1,r2,g2,b2) \
	    (long) (3L*(long)((r1)-(r2))*(long)((r1)-(r2)) + \
		    4L*(long)((g1)-(g2))*(long)((g1)-(g2)) + \
		    2L*(long)((b1)-(b2))*(long)((b1)-(b2)))


static unsigned char masktable[] = { 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01 };



CDIB::CDIB(HANDLE hDib,int nBits)
{
	m_pVoid = NULL;
	m_pLinePtr = NULL;
	m_bUseGamma=FALSE;
	width=height=0;
	if(hDib)
	{
		CreateFromHandle(hDib,nBits);
	}
}

CDIB::~CDIB()
{
	DestroyDIB();
}

void CDIB::DestroyDIB()
{
	if(m_pVoid) free(m_pVoid);
	m_pVoid = NULL;
	if(m_pLinePtr) free(m_pLinePtr);
	m_pLinePtr = NULL;
}


BOOL CDIB::Create(int width,int height,int bits)
{
	/*
		Free existing image
	*/
	DestroyDIB();
//	ASSERT(bits == 24 || bits == 8);

BITMAPINFOHEADER bmInfo;
	
	memset(&bmInfo,0,sizeof(BITMAPINFOHEADER));
	bmInfo.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo.biWidth = width;
	bmInfo.biHeight = height;
	bmInfo.biPlanes = 1;
	bmInfo.biBitCount = bits;
	bmInfo.biCompression = BI_RGB;
	return Create(bmInfo);
}

BOOL CDIB::Create(BITMAPINFOHEADER& bmInfo)
{
	bytes = (bmInfo.biBitCount*bmInfo.biWidth)>>3;
	height = bmInfo.biHeight;
	width = bmInfo.biWidth;
//	bmInfo.biHeight *= -1;
	while(bytes%4) bytes++;

	int size;
	size = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteSize(bmInfo) + bytes*height;
	m_pVoid = (void *)malloc(size);
	if(!m_pVoid) return FALSE;

	m_pInfo = (PBITMAPINFO )m_pVoid;
	memcpy((void *)&m_pInfo->bmiHeader,(void *)&bmInfo,sizeof(BITMAPINFOHEADER));
	m_pRGB = (RGBQUAD *)((unsigned char *)m_pVoid + sizeof(BITMAPINFOHEADER)) ;
	m_pBits = (unsigned char *)(m_pVoid) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteSize();

int i;
BYTE **ptr;
	m_pLinePtr = (BYTE **)malloc(sizeof(BYTE *)*height);
	if(!m_pLinePtr) return FALSE;
	for(i=0,ptr=m_pLinePtr; i < height; i++,ptr++)
	{
		//*ptr = (int)(m_pBits)+(i*bytes);
		//*ptr = (int)GetLinePtr(i);
		*ptr = m_pBits + (height-i-1)*bytes;
	}
	m_nFlags = 0;
	return TRUE;
}

void CDIB::SetPalette(unsigned char *palette)
{
int i,size;
RGBQUAD *rgb;
	if(!palette) return;
	size = GetPaletteSize();
	for(i=0,rgb = m_pRGB; i < size; i++,rgb++,palette+=3)
	{
		if(m_bUseGamma)
		{
			rgb->rgbRed = Gamma[palette[0]];
			rgb->rgbGreen = Gamma[palette[1]];
			rgb->rgbBlue = Gamma[palette[2]];
		}
		else
		{
			rgb->rgbRed = palette[0];
			rgb->rgbGreen = palette[1];
			rgb->rgbBlue = palette[2];
		}
		rgb->rgbReserved = (BYTE)0;
	}
}

void CDIB::SetPalette(RGBQUAD *pRGB)
{
int size;
	if(!pRGB) return;
	size = GetPaletteSize();
	memcpy(m_pRGB,pRGB,size*sizeof(RGBQUAD));
}


int CDIB::GetPaletteSize()
{
	return GetPaletteSize(m_pInfo->bmiHeader);
}


int CDIB::GetPaletteSize(BITMAPINFOHEADER& bmInfo)
{
	switch(bmInfo.biBitCount)
	{
	case 1:
			return 2;
	case 4:
			return 16;
	case 8:
			return 256;
	default:
			return 0;
	}
}


void CDIB::SetPixel(int x,int y,COLORREF color)
{
unsigned char *ptr;
	ASSERT(x >= 0 && y >=0);
	ASSERT(x < width && y < height);

//	ptr = m_pBits + (y*bytes) + x * 3;
	ptr = (unsigned char *)m_pLinePtr[y];
	ptr += x*3;
	*ptr++ = (unsigned char)GetBValue(color);
	*ptr++ = (unsigned char)GetGValue(color);
	*ptr++ = (unsigned char)GetRValue(color);
}

void CDIB::SetPixel8(int x,int y,unsigned char color)
{
unsigned char *ptr,*aptr;
	ASSERT(x >= 0 && y >=0);
	ASSERT(x < width && y < height);

//	ptr = m_pBits + (y*bytes) + x ;
//	ptr = (unsigned char *)m_pLinePtr[y] ;
	ptr = GetLinePtr(y);
	aptr = ptr;
	ptr += x;
	*ptr = color;
}


COLORREF CDIB::GetPixel(int x,int y)
{
unsigned char *ptr;
COLORREF color;
	ASSERT(x >= 0 && y >=0);
	ASSERT(x < width && y < height);

//	ptr = m_pBits + (y*bytes) + x * 3;
	ptr = GetLinePtr(y);
	ptr += (x*3);
	color = RGB(*(ptr+2),*(ptr+1),*ptr);
	return color;
}

CBitmap *CDIB::GetTempBitmap(CDC& dc)
{
HBITMAP hBitmap;
CBitmap *temp;
	ASSERT(m_pVoid != NULL);
	hBitmap = CreateDIBitmap(dc.m_hDC,
				(PBITMAPINFOHEADER)m_pInfo,
				CBM_INIT,
				(const void *)m_pBits,
				m_pInfo,
				DIB_RGB_COLORS);

	if(hBitmap == NULL) return NULL;
	temp = CBitmap::FromHandle(hBitmap);
	return temp;
}

CBitmap *CDIB::GetBitmap(CDC& dc)
{
HBITMAP hBitmap;
CBitmap *temp;
	ASSERT(m_pVoid != NULL);
	hBitmap = CreateDIBitmap(dc.m_hDC,
				(PBITMAPINFOHEADER)m_pInfo,
				CBM_INIT,
				(const void *)m_pBits,
				m_pInfo,
				DIB_RGB_COLORS);

	if(hBitmap == NULL) return NULL;
	temp = CBitmap::FromHandle(hBitmap);
	if(temp)
	{
		BITMAP bmp;
		LPVOID lpVoid;
		temp->GetBitmap(&bmp);
		lpVoid = malloc(bmp.bmWidthBytes*bmp.bmHeight);
		if(!lpVoid) return NULL;
		temp->GetBitmapBits(bmp.bmWidthBytes*bmp.bmHeight,lpVoid);
		CBitmap *newBmp = new CBitmap;
		newBmp->CreateBitmapIndirect(&bmp);
		newBmp->SetBitmapBits(bmp.bmWidthBytes*bmp.bmHeight,lpVoid);
		free(lpVoid);
		return newBmp;
	}
	else return NULL;

}

void CDIB::CopyLine(int source,int dest)
{
unsigned char *src,*dst;
	ASSERT(source <= height && source >= 0);
	ASSERT(dest <= height && dest >= 0);
	if(source == dest) return;
	src = GetLinePtr(source);
	dst = GetLinePtr(dest);
	memcpy(dst,src,bytes);
}

void CDIB::InitDIB(COLORREF color)
{
int i,j;
unsigned char *ptr;
	
	if(m_pInfo->bmiHeader.biBitCount == 24)
	{
		unsigned char col[3];
		col[0]=GetBValue(color);
		col[1]=GetGValue(color);
		col[2]=GetRValue(color);
		for(i=0,ptr = m_pBits; i < height; i++)
		{
			ptr = m_pBits + i*bytes;
			for(j=0; j < width ; j++,ptr+=3)
			{
				memcpy(ptr,col,3);
			}
		}
	}
	else
	{
		for(i=0,ptr = m_pBits; i < height; i++,ptr+=bytes)
		{
			memset(ptr,(BYTE)color,bytes);
		}
	}
}


void CDIB::BitBlt(HDC hDest,int nXDest,int nYDest,int nWidth,int nHeight,int xSrc,int ySrc)
{
	SetDIBitsToDevice(hDest,nXDest,nYDest,nWidth,nHeight,xSrc,Height()-ySrc-nHeight,0,Height(),m_pBits,m_pInfo,DIB_RGB_COLORS);
}

void CDIB::StretchBlt(HDC hDest,int nXDest,int nYDest,int nDWidth,int nDHeight,int xSrc,int ySrc,int  nSWidth,int nSHeight)
{
	int err;
	err = StretchDIBits(hDest,nXDest,nYDest,nDWidth,nDHeight,xSrc,ySrc,nSWidth,nSHeight,m_pBits,(CONST BITMAPINFO * )&m_pInfo->bmiHeader,DIB_RGB_COLORS,SRCCOPY);
}

void CDIB::ExpandBlt(int nXDest,int nYDest,int xRatio,int yRatio,CDIB& dibSrc,int xSrc,int ySrc,int  nSWidth,int nSHeight)
{
	SetPalette(dibSrc.m_pRGB);

	nSWidth = xSrc+nSWidth > dibSrc.width ? dibSrc.width-xSrc : nSWidth;
	nSHeight = ySrc+nSHeight > dibSrc.height? dibSrc.height-ySrc : nSHeight;

	Expand(nXDest,nYDest,xRatio,yRatio,dibSrc,xSrc,ySrc,nSWidth,nSHeight);
}

void CDIB::Expand(int nXDest,int nYDest,int xRatio,int yRatio,CDIB& dibSrc,int xSrc,int ySrc,int  nSWidth,int nSHeight)
{
int xNum,yNum,xErr,yErr;	
int nDWidth,nDHeight;
	
	nDWidth = nSWidth*xRatio;
	nDHeight = nSHeight*yRatio;

	nDWidth = nXDest+nDWidth > width ? width-nXDest : nDWidth ; 
	nDHeight = nYDest+nDHeight > height ? height-nYDest : nDHeight;

	xNum = nDWidth/xRatio;
	yNum = nDHeight/yRatio;
	xErr = nDWidth%xRatio;
	yErr = nDHeight%yRatio;

unsigned char *buffer,*srcPtr,*destPtr,*ptr;
int i,j,k;
	
	buffer = (unsigned char *)malloc(nDWidth+20);
	if(!buffer) return;

	for(i=0; i < yNum; i++,ySrc++)
	{
		srcPtr = dibSrc.GetLinePtr(ySrc) + xSrc;
		ptr = buffer;
		for(j=0; j < xNum; j++,ptr+=xRatio)
		{
			memset(ptr,*(srcPtr+j),xRatio);
			k=*(srcPtr+j);
		}
		memset(ptr,(unsigned char)k,xErr);
		for(j=0; j < yRatio ; j++,nYDest++)
		{
			destPtr = GetLinePtr(nYDest) + nXDest;
			memcpy(destPtr,buffer,nDWidth);		
		}
	}
	for(j=0; j < yErr; j++,nYDest++)
	{
		destPtr = GetLinePtr(nYDest) + nXDest;
		memcpy(destPtr,buffer,nDWidth);		
	}
	free(buffer);
}

void CDIB::StretchBlt(int nXDest,int nYDest,int nDWidth,int nDHeight,CDIB& dibSrc,int xSrc,int ySrc,int  nSWidth,int nSHeight)
{
	SetPalette(dibSrc.m_pRGB);
	nDWidth = nXDest+nDWidth > width ? width-nXDest : nDWidth ; 
	nDHeight = nYDest+nDHeight > height ? height-nYDest : nDHeight;

	nSWidth = xSrc+nSWidth > dibSrc.width ? dibSrc.width-xSrc : nSWidth;
	nSHeight = ySrc+nSHeight > dibSrc.height? dibSrc.height-ySrc : nSHeight;

int xDiv,yDiv;
int xMod,yMod;

	xDiv = nDWidth/nSWidth;
	xMod = nDWidth%nSWidth;

	yDiv = nDHeight/nSHeight;
	yMod = nDHeight%nSHeight;

	if(!xMod && !yMod && xDiv > 0 && yDiv > 0)
	{
		ExpandBlt(nXDest,nYDest,xDiv,yDiv,dibSrc,xSrc,ySrc,nSWidth,nSHeight);
		return;
	}

unsigned char *tempPtr,*srcPix,*destPix,*q;
	tempPtr = (unsigned char *)malloc(nDWidth+20);
int i,j,k,l,x,y,m;
int xErr,yErr;
	for(i=yErr=m=0; i < nSHeight; i++)
	{
		srcPix = dibSrc.GetLinePtr(i+ySrc) + xSrc;
		q = tempPtr;
		for(j=l=xErr=0; j < nSWidth; j++,srcPix++)
		{
			k = xDiv;
			xErr += xMod;
			if(xErr >= nSWidth)
			{
				k++;
				xErr%=nSWidth;
			}
			x=0;
			while(l < nDWidth &&  x < k)
			{
				*q++ = *srcPix;
				l++;
				x++;
			}
		}
		while(l < nDWidth)
		{
			*q++=*srcPix;
			l++;
		}
		k= yDiv;
		yErr += yMod;
		if(yErr >= nSHeight)
		{
			k++;
			yErr%=nSHeight;
		}
		y=0;
		while(m < nDHeight && y < k)
		{
			destPix = GetLinePtr(m+nYDest) + nXDest;
			memcpy(destPix,tempPtr,nDWidth);
			m++;
			y++;
		}
	}
	while(m < nDHeight )
	{
		destPix = GetLinePtr(m+nYDest) + nXDest;
		memcpy(destPix,tempPtr,nDWidth);
		m++;
	}
	free(tempPtr);
}

void CDIB::BitBlt(int nXDest,int nYDest,int nWidth,int nHeight,CDIB& dibSrc,int nSrcX,int nSrcY,BYTE *colors)
{
	SetPalette(dibSrc.m_pRGB);
	if(nXDest < 0)
	{
		nSrcX -= nXDest;
		nWidth += nXDest;
		nXDest=0;
	}
	if(nYDest < 0)
	{
		nSrcY -= nYDest;
		nHeight += nYDest;
		nYDest=0;
	}
	if(nSrcX < 0)
	{
		nXDest -= nSrcX;
		nWidth += nSrcX;
		nSrcX=0;
	}
	if(nSrcY < 0)
	{
		nYDest -= nSrcY;
		nHeight += nSrcY;
		nSrcY=0;
	}
	nWidth = nXDest+nWidth > width ? width-nXDest : nWidth ; 
	nHeight = nYDest+nHeight > height ? height-nYDest : nHeight;

	nWidth = nSrcX+nWidth > dibSrc.width ? dibSrc.width-nSrcX : nWidth;
	nHeight = nSrcY+nHeight > dibSrc.height? dibSrc.height-nSrcY : nHeight;

	nWidth = __max(0,nWidth);
	nHeight = __max(0,nHeight);
int i,k,l,j;
unsigned char *srcPtr,*destPtr;
	if(!colors)
	{
		for(i=0,k=nSrcY,l=nYDest; i < nHeight; i++,k++,l++)
		{
			if(k < 0 || l < 0)
			{
				continue;
			}
			else
			{
				srcPtr = dibSrc.GetLinePtr(k);
				destPtr = GetLinePtr(l);
				memcpy(destPtr+nXDest,srcPtr+nSrcX,nWidth);
			}
		}
	}
	else
	{
		for(i=0,k=nSrcY,l=nYDest; i < nHeight; i++,k++,l++)
		{
			if(k < 0 || l < 0)
			{
				continue;
			}
			else
			{
				srcPtr = dibSrc.GetLinePtr(k)+nXDest;
				destPtr = GetLinePtr(l)+nSrcX;
				for(j=0; j < nWidth; j++,srcPtr++,destPtr++)
				{
					if(colors[*srcPtr]) *destPtr=*srcPtr;
				}
			}
		}
	}
}

unsigned char *CDIB::GetLinePtr(int line)
{
/*unsigned char *ptr;
	ptr = m_pBits + (height-line-1)*bytes;
	return ptr;*/
	return m_pLinePtr[line];
}
	
BOOL CDIB::CopyDIB(CDIB& dib)
{
	if(Create(dib.m_pInfo->bmiHeader))
	{
		SetPalette(dib.m_pRGB);
		memcpy(m_pBits,dib.m_pBits,height*bytes);
		return TRUE;
	}
	return FALSE;
}

void CDIB::ReplaceColor(unsigned char oldColor,unsigned char newColor)
{
int i,j;
unsigned char *ptr;	
	for(i=0; i < height; i++)
	{
		ptr = GetLinePtr(i);
		for(j=0; j < width; j++)
		{
			if(ptr[j] == oldColor) ptr[j] = newColor;
		}
	}
}


CDIB& CDIB::operator=(CDIB& dib)
{
	CopyDIB(dib);
	return *this;
}

HANDLE CDIB::GetDIBits(int nStartX,int nStartY,int nCx,int nCy)
{
	if(nStartX == -1)
	{
		nStartX = nStartY=0;
		nCx = width;
		nCy = height;
		CDIB dib;
		dib.Create(nCx,nCy,8);
		dib.BitBlt(0,0,nCx,nCy,*this,0,0);
		dib.SetPalette(m_pRGB);
		return dib.DIBHandle();
	}
	return DIBHandle();
}

DWORD CDIB::GetDIBSize()
{
	return sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteSize() + bytes*height;
}

HANDLE CDIB::DIBHandle()
{
int nSize;
HANDLE hMem;
	nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteSize() + bytes*height;
	hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,nSize);
	if(hMem  == NULL) return NULL;
UCHAR *lpVoid,*pBits;
LPBITMAPINFOHEADER pHead;
RGBQUAD *pRgb;
	lpVoid = (UCHAR *)GlobalLock(hMem);
	pHead = (LPBITMAPINFOHEADER )lpVoid;
	memcpy(pHead,&m_pInfo->bmiHeader,sizeof(BITMAPINFOHEADER));
	pRgb = (RGBQUAD *)(lpVoid + sizeof(BITMAPINFOHEADER) );
	memcpy(pRgb,m_pRGB,sizeof(RGBQUAD)*GetPaletteSize());
	pBits = lpVoid + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteSize();
	memcpy(pBits,m_pBits,height*bytes);
	GlobalUnlock(lpVoid);
	return hMem;
}

BOOL CDIB::CreateFromHandle(HANDLE hMem,int bits)
{
	DestroyDIB();
UCHAR *lpVoid,*pBits;
LPBITMAPINFOHEADER pHead;
RGBQUAD *pRgb;
	lpVoid = (UCHAR *)GlobalLock(hMem);
	pHead = (LPBITMAPINFOHEADER )lpVoid;
	width = pHead->biWidth;
	height = pHead->biHeight;
	m_nBits = pHead->biBitCount;
	if(pHead->biCompression != BI_RGB) 
	{
		GlobalUnlock(lpVoid);
		return FALSE;
	}
	if(pHead->biBitCount >= 15)
	{
		if(pHead->biBitCount != 24) 
		{
			GlobalUnlock(lpVoid);
			return FALSE;
		}
	}
	if(!Create(*pHead))
	{
		GlobalUnlock(lpVoid);
		return FALSE;
	}
	pRgb = (RGBQUAD *)(lpVoid + sizeof(BITMAPINFOHEADER) );
	memcpy(m_pRGB,pRgb,sizeof(RGBQUAD)*GetPaletteSize());
	pBits = lpVoid + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteSize();
	memcpy(m_pBits,pBits,height*bytes);
	GlobalUnlock(lpVoid);
	return TRUE;
}

void CDIB::UseGamma(float fg,BOOL bUse)
{
	m_bUseGamma = bUse;
	m_fOldGamma = m_fGamma;
	m_fGamma = fg;
	CreateGammaCurve();
}


void CDIB::CreateGammaCurve()
{
int i;
	for(i=0;i<256;++i)
	{
	    Gamma[i]=(int)(255 * powf((double)i/255,m_fGamma) + (double)0.5);
	}
}



void CDIB::GetPixel(UINT x,UINT y,int& pixel)
{
	ASSERT(x < (UINT)Width());
	ASSERT(y < (UINT)Height());
	if(x >= (UINT)Width()) return;
	if(y >= (UINT)Height()) return;
	pixel=(GetLinePtr(y))[x];
}

BOOL CDIB::Make8Bit(CDIB& dib)
{
int nBits;	
	ASSERT(Width() == dib.Width());
	ASSERT(Height() == dib.Height());
	nBits = dib.GetBitCount();
	switch(nBits)
	{
	case 1:
		return SwitchFromOne(dib);
		break;
	case 4:
		return SwitchFromFour(dib);
		break;
	case 8:
		return SwitchPalette(dib);
		break;
	case 24:
		return SwitchFrom24(dib);
		break;
	default:
		return FALSE;
	}
	return FALSE;
}

/*
BOOL CDIB::SwitchFrom24(CDIB& dib)
{
int i,j,w,h;
unsigned char *sPtr,*dPtr;
	w = Width();
	h = Height();
	memset(CachePtr,0,sizeof(CachePtr));
	for(i=0; i < h; i++)
	{
		dPtr = GetLinePtr(i);
		sPtr = dib.GetLinePtr(i);
		for(j=0 ; j < w; j++,dPtr++,sPtr+=3)
		{
			*dPtr = ClosestColor((RGBQUAD *)sPtr);
		}
	}
	return TRUE;
}
*/


BOOL CDIB::SwitchFromOne(CDIB& dib)
{
int i,j,w,h;
unsigned char *sPtr,*dPtr;
unsigned char cols[2];
	w = Width();
	h = Height();
	memset(CachePtr,0,sizeof(CachePtr));
	cols[0]=ClosestColor(dib.m_pRGB);
	cols[1]=ClosestColor(dib.m_pRGB+1);
	for(i=0; i < h; i++)
	{
		dPtr = GetLinePtr(i);
		sPtr = dib.GetLinePtr(i);
		for(j=0 ; j < w; j++,dPtr++)
		{
			if(!(sPtr[j>>3] & masktable[j&7])) *dPtr = cols[0];
			else *dPtr = cols[1];
		}
	}
	return TRUE;
}

BOOL CDIB::SwitchFromFour(CDIB& dib)
{
int i,n,j,w,h;
unsigned char *sPtr,*dPtr;
unsigned char cols[16];
	w = Width();
	h = Height();
	memset(CachePtr,0,sizeof(CachePtr));
	for(i=0; i < 16; i++)
	{
		cols[i]=ClosestColor(dib.m_pRGB+i);
	}
	for(i=0; i < h; i++)
	{
		dPtr = GetLinePtr(i);
		sPtr = dib.GetLinePtr(i);
		for(j=0 ; j < w; j++,dPtr++)
		{
			if(!(j&1)) n = (*sPtr & 0xf0)>>4;
			else 
			{
				n = *sPtr & 0x0f;
				sPtr++;
			}
			*dPtr = cols[n];
		}
	}
	return TRUE;
}

BOOL CDIB::SwitchPalette(CDIB& dib)
{
int i,j,w,h;
unsigned char *sPtr,*dPtr;
unsigned char cols[256];
	w = Width();
	h = Height();
	memset(CachePtr,0,sizeof(CachePtr));
	for(i=0; i < 256; i++)
	{
		cols[i]=ClosestColor(dib.m_pRGB+i);
	}
	for(i=0; i < h; i++)
	{
		dPtr = GetLinePtr(i);
		sPtr = dib.GetLinePtr(i);
		for(j=0 ; j < w; j++,sPtr++,dPtr++)
		{
			*dPtr = cols[*sPtr];
		}
	}
	return TRUE;
}


int CDIB::ClosestColor(RGBQUAD *pRgb)
{
unsigned int dist=BIG_DISTANCE,i,d,c;
RGBQUAD *pQuad=m_pRGB;
unsigned int pSize=GetPaletteSize();
	for(i=0; i < pSize;i++)
	{
		if(CachePtr[i])
		{
			if(!memcmp((void *)&CacheQuad[i],(void *)pRgb,3)) 
			{
				return i;
			}
		}
	}
	for(i=0; i < pSize; i++,pQuad++)
	{
		d = Distance(*pRgb,*pQuad);
		if(!d) 
		{
			CacheQuad[i]=*pRgb;
			CachePtr[i]=1;
			return i;
		}		
		if(dist > d) 
		{
			c = i;
			dist = d;
		}
	}
	CacheQuad[c]=*pRgb;
	CachePtr[c]=1;
	return c;
}

unsigned int CDIB::Distance(RGBQUAD& rgb1,RGBQUAD& rgb2)
{
unsigned int d;
	d =  3*(unsigned)((rgb1.rgbRed)-(rgb2.rgbRed))*(unsigned)((rgb1.rgbRed)-(rgb2.rgbRed));
	d += 4*(unsigned)((rgb1.rgbGreen)-(rgb2.rgbGreen))*(unsigned)((rgb1.rgbGreen)-(rgb2.rgbGreen)) ;
	d += 2*(unsigned)((rgb1.rgbBlue)-(rgb2.rgbBlue))*(unsigned)((rgb1.rgbBlue)-(rgb2.rgbBlue));
	return d;
}

BOOL CDIB::OpenDIB(CString& csFileName)
{
CFile file;
	if(!file.Open(csFileName,CFile::modeRead | CFile::typeBinary))
	{
		return FALSE;
	}
	file.Close();
	if(OpenBMP(csFileName)) return TRUE;
	return FALSE;
}



BOOL CDIB::SaveDIB(CString& csFileName,BitmapType type)
{
CFile file;
	if(!file.Open(csFileName,CFile::modeCreate | CFile::typeBinary))
	{
		return FALSE;
	}
	file.Close();
	switch(type)
	{
	case BMP:
			return SaveBMP(csFileName);
	default:
			return FALSE;
	}
	return FALSE;
}

BOOL CDIB::SaveBMP(CString& csFileName)
{
BITMAPFILEHEADER bFile;
CFile file;
	if(!file.Open(csFileName,CFile::modeWrite | CFile::typeBinary))
	{
		return FALSE;
	}
	::ZeroMemory(&bFile,sizeof(bFile));
	memcpy((void *)&bFile.bfType,"BM",2);
	bFile.bfSize = GetDIBSize() + sizeof(bFile);
	bFile.bfOffBits = sizeof(BITMAPINFOHEADER) + GetPaletteSize()*sizeof(RGBQUAD) + sizeof(BITMAPFILEHEADER);
	file.Write(&bFile,sizeof(bFile));
	file.Write(m_pVoid,GetDIBSize());
	file.Close();
	return TRUE;

}

BOOL CDIB::OpenBMP(CString& csFileName)
{
BITMAPFILEHEADER bFile;
BITMAPINFOHEADER head;
CFile file;
	if(!file.Open(csFileName,CFile::modeRead | CFile::typeBinary))
	{
		return FALSE;
	}
	file.Read(&bFile,sizeof(bFile));
	if(memcmp((void *)&bFile.bfType,"BM",2))
	{
		file.Close();
		return FALSE;
	}
	file.Read(&head,sizeof(head));
	if(!Create(head))
	{
		file.Close();
		return FALSE;
	}
	file.Read(m_pRGB,sizeof(RGBQUAD)*GetPaletteSize());
	file.Seek(bFile.bfOffBits,CFile::begin);
	file.Read(m_pBits,height*bytes);
	file.Close();
	return TRUE;

}


int CDIB::CountColors()
{
	ASSERT(GetBitCount()==8);
BYTE colors[256],*ptr;
int nNum=0,i,j,w,d;
	w = Width();
	d = Height();
	memset(colors,0,256);
	for(i=0; i < d; i++)
	{
		ptr = GetLinePtr(i);
		for(j=0; j < w; j++,ptr++)
		{
			if(!colors[*ptr])
			{
				colors[*ptr]=1;
				nNum++;
			}
		}
	}
	return nNum;
}

int CDIB::EnumColors(BYTE *array)
{
	ASSERT(GetBitCount()==8);
BYTE *ptr;
int nNum=0,i,j,w,d;
	w = Width();
	d = Height();
	memset(array,0,256);
	for(i=0; i < d; i++)
	{
		ptr = GetLinePtr(i);
		for(j=0; j < w; j++,ptr++)
		{
			if(!array[*ptr])
			{
				array[*ptr]=1;
				nNum++;
			}
		}
	}
	return nNum;
}

COLORREF CDIB::PaletteColor(int nIndex)
{
	ASSERT(nIndex < 256);
RGBQUAD *pRGB= m_pRGB+nIndex;
	return RGB(pRGB->rgbRed,pRGB->rgbGreen,pRGB->rgbBlue);
}

BOOL CDIB::SwitchFrom24(CDIB& dib)
{
int i,j,w,h,c;
unsigned char *sPtr,*dPtr;
BYTE *index_ptr=NULL;
RGBQUAD rgb;
	w = Width();
	h = Height();
	index_ptr = (BYTE *)malloc(0x7FFF+1);
	if(!index_ptr) return FALSE;
	memset(CachePtr,0,sizeof(CachePtr));
	for(i=0; i <= 0x7FFF; i++)
	{
		rgb.rgbRed = (((i & 0x7C00)>>10) << 3) | 0x07;
		rgb.rgbGreen = (((i & 0x3e0)>>5) << 3) | 0x07;
		rgb.rgbBlue = ((i & 0x1F)<<3) | 0x07;
		index_ptr[i] = ClosestColor(&rgb);
	}
	for(i=0; i < h; i++)
	{
		dPtr = GetLinePtr(i);
		sPtr = dib.GetLinePtr(i);
		for(j=0 ; j < w; j++,dPtr++,sPtr+=3)
		{
			c = (*sPtr >> 3) | ((*(sPtr+1) >> 3) << 5) | ((*(sPtr+2) >> 3) << 10);
			*dPtr = index_ptr[c];
		}
	}
	free(index_ptr);
	return TRUE;
}

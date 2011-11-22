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

#include "tr_local.h"

/*
================
R_ResampleTexture

Used to resample images in a more general than quartering fashion.

This will only have filter coverage if the resampled size
is greater than half the original size.

If a larger shrinking is needed, use the mipmap function 
after resampling to the next lower power of two.
================
*/
#define	MAX_DIMENSION	4096
byte *R_ResampleTexture( const byte *in, int inwidth, int inheight,  
							int outwidth, int outheight ) {
	int		i, j;
	const byte	*inrow, *inrow2;
	unsigned int	frac, fracstep;
	unsigned int	p1[MAX_DIMENSION], p2[MAX_DIMENSION];
	const byte		*pix1, *pix2, *pix3, *pix4;
	byte		*out, *out_p;

	if ( outwidth > MAX_DIMENSION ) {
		outwidth = MAX_DIMENSION;
	}
	if ( outheight > MAX_DIMENSION ) {
		outheight = MAX_DIMENSION;
	}

	out = (byte *)R_StaticAlloc( outwidth * outheight * 4 );
	out_p = out;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for ( i=0 ; i<outwidth ; i++ ) {
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for ( i=0 ; i<outwidth ; i++ ) {
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out_p += outwidth*4 ) {
		inrow = in + 4 * inwidth * (int)( ( i + 0.25f ) * inheight / outheight );
		inrow2 = in + 4 * inwidth * (int)( ( i + 0.75f ) * inheight / outheight );
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++) {
			pix1 = inrow + p1[j];
			pix2 = inrow + p2[j];
			pix3 = inrow2 + p1[j];
			pix4 = inrow2 + p2[j];
			out_p[j*4+0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			out_p[j*4+1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			out_p[j*4+2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			out_p[j*4+3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}

	return out;
}

/*
================
R_Dropsample

Used to resample images in a more general than quartering fashion.
Normal maps and such should not be bilerped.
================
*/
byte *R_Dropsample( const byte *in, int inwidth, int inheight,  
							int outwidth, int outheight ) {
	int		i, j, k;
	const byte	*inrow;
	const byte	*pix1;
	byte		*out, *out_p;

	out = (byte *)R_StaticAlloc( outwidth * outheight * 4 );
	out_p = out;

	for (i=0 ; i<outheight ; i++, out_p += outwidth*4 ) {
		inrow = in + 4*inwidth*(int)((i+0.25)*inheight/outheight);
		for (j=0 ; j<outwidth ; j++) {
			k = j * inwidth / outwidth;
			pix1 = inrow + k * 4;
			out_p[j*4+0] = pix1[0];
			out_p[j*4+1] = pix1[1];
			out_p[j*4+2] = pix1[2];
			out_p[j*4+3] = pix1[3];
		}
	}

	return out;
}


/*
===============
R_SetBorderTexels

===============
*/
void R_SetBorderTexels( byte *inBase, int width, int height, const byte border[4] ) {
	int		i;
	byte	*out;

	out = inBase;
	for (i=0 ; i<height ; i++, out+=width*4) {
		out[0] = border[0];
		out[1] = border[1];
		out[2] = border[2];
		out[3] = border[3];
	}
	out = inBase+(width-1)*4;
	for (i=0 ; i<height ; i++, out+=width*4) {
		out[0] = border[0];
		out[1] = border[1];
		out[2] = border[2];
		out[3] = border[3];
	}
	out = inBase;
	for (i=0 ; i<width ; i++, out+=4) {
		out[0] = border[0];
		out[1] = border[1];
		out[2] = border[2];
		out[3] = border[3];
	}
	out = inBase+width*4*(height-1);
	for (i=0 ; i<width ; i++, out+=4) {
		out[0] = border[0];
		out[1] = border[1];
		out[2] = border[2];
		out[3] = border[3];
	}
}

/*
===============
R_SetBorderTexels3D

===============
*/
void R_SetBorderTexels3D( byte *inBase, int width, int height, int depth, const byte border[4] ) {
	int		i, j;
	byte	*out;
	int		row, plane;

	row = width * 4;
	plane = row * depth;

	for ( j = 1 ; j < depth - 1 ; j++ ) {
		out = inBase + j * plane;
		for (i=0 ; i<height ; i++, out+=row) {
			out[0] = border[0];
			out[1] = border[1];
			out[2] = border[2];
			out[3] = border[3];
		}
		out = inBase+(width-1)*4 + j * plane;
		for (i=0 ; i<height ; i++, out+=row) {
			out[0] = border[0];
			out[1] = border[1];
			out[2] = border[2];
			out[3] = border[3];
		}
		out = inBase + j * plane;
		for (i=0 ; i<width ; i++, out+=4) {
			out[0] = border[0];
			out[1] = border[1];
			out[2] = border[2];
			out[3] = border[3];
		}
		out = inBase+width*4*(height-1) + j * plane;
		for (i=0 ; i<width ; i++, out+=4) {
			out[0] = border[0];
			out[1] = border[1];
			out[2] = border[2];
			out[3] = border[3];
		}
	}

	out = inBase;
	for ( i = 0 ; i < plane ; i += 4, out += 4 ) {
		out[0] = border[0];
		out[1] = border[1];
		out[2] = border[2];
		out[3] = border[3];
	}
	out = inBase+(depth-1)*plane;
	for ( i = 0 ; i < plane ; i += 4, out += 4 ) {
		out[0] = border[0];
		out[1] = border[1];
		out[2] = border[2];
		out[3] = border[3];
	}
}

/*
================
R_SetAlphaNormalDivergence

If any of the angles inside the cone would directly reflect to the light, there will be
a specular highlight.  The intensity of the highlight is inversely proportional to the
area of the spread.

Light source area is important for the base size.

area subtended in light is the divergence times the distance

Shininess value is subtracted from the divergence

Sets the alpha channel to the greatest divergence dot product of the surrounding texels.
1.0 = flat, 0.0 = turns a 90 degree angle
Lower values give less shiny specular
With mip maps, the lowest samnpled value will be retained

Should we rewrite the normal as the centered average?
================
*/
void	R_SetAlphaNormalDivergence( byte *in, int width, int height ) {
	for ( int y = 0 ; y < height ; y++ ) {
		for ( int x = 0 ; x < width ; x++ ) {
			// the divergence is the smallest dot product of any of the eight surrounding texels
			byte	*pic_p = in + ( y * width + x ) * 4;
			idVec3	center;
			center[0] = ( pic_p[0] - 128 ) / 127;
			center[1] = ( pic_p[1] - 128 ) / 127;
			center[2] = ( pic_p[2] - 128 ) / 127;
			center.Normalize();

			float	maxDiverge = 1.0;

			// FIXME: this assumes wrap mode, but should handle clamp modes and border colors
			for ( int yy = -1 ; yy <= 1 ; yy++ ) {
				for ( int xx = -1 ; xx <= 1 ; xx++ ) {
					if ( yy == 0 && xx == 0 ) {
						continue;
					}
					byte	*corner_p = in + ( ((y+yy)&(height-1)) * width + ((x+xx)&width-1) ) * 4;
					idVec3	corner;
					corner[0] = ( corner_p[0] - 128 ) / 127;
					corner[1] = ( corner_p[1] - 128 ) / 127;
					corner[2] = ( corner_p[2] - 128 ) / 127;
					corner.Normalize();

					float	diverge = corner * center;
					if ( diverge < maxDiverge ) {
						maxDiverge = diverge;
					}
				}
			}

			// we can get a diverge < 0 in some extreme cases
			if ( maxDiverge < 0 ) {
				maxDiverge = 0;
			}
			pic_p[3] = maxDiverge * 255;
		}
	}
}

/*
================
R_MipMapWithAlphaSpecularity

Returns a new copy of the texture, quartered in size and filtered.
The alpha channel is taken to be the minimum of the dots of all surrounding normals.
================
*/
#define MIP_MIN(a,b) (a<b?a:b)

byte *R_MipMapWithAlphaSpecularity( const byte *in, int width, int height ) {
	int		i, j, c, x, y, sx, sy;
	const byte	*in_p;
	byte	*out, *out_p;
	int		row;
	int		newWidth, newHeight;
	float	*fbuf, *fbuf_p;

	if ( width < 1 || height < 1 || ( width + height == 2 ) ) {
		common->FatalError( "R_MipMapWithAlphaMin called with size %i,%i", width, height );
	}

	// convert the incoming texture to centered floating point
	c = width * height;
	fbuf = (float *)_alloca( c * 4 * sizeof( *fbuf ) );
	in_p = in;
	fbuf_p = fbuf;
	for ( i = 0 ; i < c ; i++, in_p+=4, fbuf_p += 4 ) {
		fbuf_p[0] = ( in_p[0] / 255.0 ) * 2.0 - 1.0;	// convert to a normal
		fbuf_p[1] = ( in_p[1] / 255.0 ) * 2.0 - 1.0;
		fbuf_p[2] = ( in_p[2] / 255.0 ) * 2.0 - 1.0;
		fbuf_p[3] = ( in_p[3] / 255.0 );				// filtered divegence / specularity
	}

	row = width * 4;

	newWidth = width >> 1;
	newHeight = height >> 1;
	if ( !newWidth ) {
		newWidth = 1;
	}
	if ( !newHeight ) {
		newHeight = 1;
	}
	out = (byte *)R_StaticAlloc( newWidth * newHeight * 4 );
	out_p = out;

	in_p = in;

	for ( i=0 ; i<newHeight ; i++ ) {
		for ( j=0 ; j<newWidth ; j++, out_p+=4 ) {
			idVec3	total;
			float	totalSpec;

			total.Zero();
			totalSpec = 0;
			// find the average normal
			for ( x = -1 ; x <= 1 ; x++ ) {
				sx = ( j * 2 + x ) & (width-1);
				for ( y = -1 ; y <= 1 ; y++ ) {
					sy = ( i * 2 + y ) & (height-1);
					fbuf_p = fbuf + ( sy * width + sx ) * 4;

					total[0] += fbuf_p[0];
					total[1] += fbuf_p[1];
					total[2] += fbuf_p[2];

					totalSpec += fbuf_p[3];
				}
			}
			total.Normalize();
			totalSpec /= 9.0;

			// find the maximum divergence
			for ( x = -1 ; x <= 1 ; x++ ) {
				for ( y = -1 ; y <= 1 ; y++ ) {
				}
			}

			// store the average normal and divergence
		}
	}

	return out;
}

/*
================
R_MipMap

Returns a new copy of the texture, quartered in size and filtered.

If a texture is intended to be used in GL_CLAMP or GL_CLAMP_TO_EDGE mode with
a completely transparent border, we must prevent any blurring into the outer
ring of texels by filling it with the border from the previous level.  This
will result in a slight shrinking of the texture as it mips, but better than
smeared clamps...
================
*/
byte *R_MipMap( const byte *in, int width, int height, bool preserveBorder ) {
	int		i, j;
	const byte	*in_p;
	byte	*out, *out_p;
	int		row;
	byte	border[4];
	int		newWidth, newHeight;

	if ( width < 1 || height < 1 || ( width + height == 2 ) ) {
		common->FatalError( "R_MipMap called with size %i,%i", width, height );
	}

	border[0] = in[0];
	border[1] = in[1];
	border[2] = in[2];
	border[3] = in[3];

	row = width * 4;

	newWidth = width >> 1;
	newHeight = height >> 1;
	if ( !newWidth ) {
		newWidth = 1;
	}
	if ( !newHeight ) {
		newHeight = 1;
	}
	out = (byte *)R_StaticAlloc( newWidth * newHeight * 4 );
	out_p = out;

	in_p = in;

	width >>= 1;
	height >>= 1;

	if ( width == 0 || height == 0 ) {
		width += height;	// get largest
		if ( preserveBorder ) {
			for (i=0 ; i<width ; i++, out_p+=4 ) {
				out_p[0] = border[0];
				out_p[1] = border[1];
				out_p[2] = border[2];
				out_p[3] = border[3];
			}
		} else {
			for (i=0 ; i<width ; i++, out_p+=4, in_p+=8 ) {
				out_p[0] = ( in_p[0] + in_p[4] )>>1;
				out_p[1] = ( in_p[1] + in_p[5] )>>1;
				out_p[2] = ( in_p[2] + in_p[6] )>>1;
				out_p[3] = ( in_p[3] + in_p[7] )>>1;
			}
		}
		return out;
	}

	for (i=0 ; i<height ; i++, in_p+=row) {
		for (j=0 ; j<width ; j++, out_p+=4, in_p+=8) {
			out_p[0] = (in_p[0] + in_p[4] + in_p[row+0] + in_p[row+4])>>2;
			out_p[1] = (in_p[1] + in_p[5] + in_p[row+1] + in_p[row+5])>>2;
			out_p[2] = (in_p[2] + in_p[6] + in_p[row+2] + in_p[row+6])>>2;
			out_p[3] = (in_p[3] + in_p[7] + in_p[row+3] + in_p[row+7])>>2;
		}
	}

	// copy the old border texel back around if desired
	if ( preserveBorder ) {
		R_SetBorderTexels( out, width, height, border );
	}

	return out;
}

/*
================
R_MipMap3D

Returns a new copy of the texture, eigthed in size and filtered.

If a texture is intended to be used in GL_CLAMP or GL_CLAMP_TO_EDGE mode with
a completely transparent border, we must prevent any blurring into the outer
ring of texels by filling it with the border from the previous level.  This
will result in a slight shrinking of the texture as it mips, but better than
smeared clamps...
================
*/
byte *R_MipMap3D( const byte *in, int width, int height, int depth, bool preserveBorder ) {
	int		i, j, k;
	const byte	*in_p;
	byte	*out, *out_p;
	int		row, plane;
	byte	border[4];
	int		newWidth, newHeight, newDepth;

	if ( depth == 1 ) {
		return R_MipMap( in, width, height, preserveBorder );
	}

	// assume symetric for now
	if ( width < 2 || height < 2 || depth < 2 ) {
		common->FatalError( "R_MipMap3D called with size %i,%i,%i", width, height, depth );
	}

	border[0] = in[0];
	border[1] = in[1];
	border[2] = in[2];
	border[3] = in[3];

	row = width * 4;
	plane = row * height;

	newWidth = width >> 1;
	newHeight = height >> 1;
	newDepth = depth >> 1;

	out = (byte *)R_StaticAlloc( newWidth * newHeight * newDepth * 4 );
	out_p = out;

	in_p = in;

	width >>= 1;
	height >>= 1;
	depth >>= 1;

	for (k=0 ; k<depth ; k++, in_p+=plane) {
		for (i=0 ; i<height ; i++, in_p+=row) {
			for (j=0 ; j<width ; j++, out_p+=4, in_p+=8) {
				out_p[0] = (in_p[0] + in_p[4] + in_p[row+0] + in_p[row+4] +
					in_p[plane+0] + in_p[plane+4] + in_p[plane+row+0] + in_p[plane+row+4]
					)>>3;
				out_p[1] = (in_p[1] + in_p[5] + in_p[row+1] + in_p[row+5] +
					in_p[plane+1] + in_p[plane+5] + in_p[plane+row+1] + in_p[plane+row+5]
					)>>3;
				out_p[2] = (in_p[2] + in_p[6] + in_p[row+2] + in_p[row+6] +
					in_p[plane+2] + in_p[plane+6] + in_p[plane+row+2] + in_p[plane+row+6]
					)>>3;
				out_p[3] = (in_p[3] + in_p[7] + in_p[row+3] + in_p[row+7] +
					in_p[plane+3] + in_p[plane+6] + in_p[plane+row+3] + in_p[plane+row+6]
					)>>3;
			}
		}
	}

	// copy the old border texel back around if desired
	if ( preserveBorder ) {
		R_SetBorderTexels3D( out, width, height, depth, border );
	}

	return out;
}


/*
==================
R_BlendOverTexture

Apply a color blend over a set of pixels
==================
*/
void R_BlendOverTexture( byte *data, int pixelCount, const byte blend[4] ) {
	int		i;
	int		inverseAlpha;
	int		premult[3];

	inverseAlpha = 255 - blend[3];
	premult[0] = blend[0] * blend[3];
	premult[1] = blend[1] * blend[3];
	premult[2] = blend[2] * blend[3];

	for ( i = 0 ; i < pixelCount ; i++, data+=4 ) {
		data[0] = ( data[0] * inverseAlpha + premult[0] ) >> 9;
		data[1] = ( data[1] * inverseAlpha + premult[1] ) >> 9;
		data[2] = ( data[2] * inverseAlpha + premult[2] ) >> 9;
	}
}


/*
==================
R_HorizontalFlip

Flip the image in place
==================
*/
void R_HorizontalFlip( byte *data, int width, int height ) {
	int		i, j;
	int		temp;

	for ( i = 0 ; i < height ; i++ ) {
		for ( j = 0 ; j < width / 2 ; j++ ) {
			temp = *( (int *)data + i * width + j );
			*( (int *)data + i * width + j ) = *( (int *)data + i * width + width - 1 - j );
			*( (int *)data + i * width + width - 1 - j ) = temp;
		}
	}
}

void R_VerticalFlip( byte *data, int width, int height ) {
	int		i, j;
	int		temp;

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height / 2 ; j++ ) {
			temp = *( (int *)data + j * width + i );
			*( (int *)data + j * width + i ) = *( (int *)data + ( height - 1 - j ) * width + i );
			*( (int *)data + ( height - 1 - j ) * width + i ) = temp;
		}
	}
}

void R_RotatePic( byte *data, int width ) {
	int		i, j;
	int		*temp;

	temp = (int *)R_StaticAlloc( width * width * 4 );

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < width ; j++ ) {
			*( temp + i * width + j ) = *( (int *)data + j * width + i );
		}
	}

	memcpy( data, temp, width * width * 4 );

	R_StaticFree( temp );
}


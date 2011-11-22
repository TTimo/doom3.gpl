
#import "roq.h"
#import "codec.h"

#ifdef __MACOS__

blah

#endif

@implementation roq


- init
{
	cWindow = eWindow = sWindow = 0;
	image = 0;
	quietMode = NO;
	encoder = 0;
	previousSize = 0;
	lastFrame = NO;
	codes = malloc( 4*1024 );
	dataStuff=NO;
	return self;
}

- (void)dealloc
{
	free( codes );

	if (image) [image dealloc];
	if (encoder) [encoder dealloc];
	return;
}

- encodeQuietly:(BOOL)which
{
	quietMode = which;
	return self;
}

- (BOOL)isQuiet
{
	return quietMode;
}

- (BOOL)isLastFrame
{
	return lastFrame;
}

- (BOOL)scaleable
{
	return [paramFileId isScaleable];
}

- (BOOL)paramNoAlpha
{
	return [paramFileId noAlpha];
}

- (BOOL)makingVideo
{
	return YES;	//[paramFileId timecode];
}

- (BOOL)searchType
{
	return	[paramFileId searchType];
}

- (BOOL)hasSound
{
	return	[paramFileId hasSound];
}

- (int)previousFrameSize
{
	return	previousSize;
}

-(int)firstFrameSize
{
	return [paramFileId firstFrameSize];
}

-(int)normalFrameSize
{
	return	[paramFileId normalFrameSize];
}

- (char *)currentFilename
{
	return currentFile;
}

-encodeStream: (id)paramInputFile
{
int onFrame;
char f0[MAXPATHLEN], f1[MAXPATHLEN], f2[MAXPATHLEN];
int morestuff;

	onFrame = 1;
	
	encoder = [[codec alloc] init: self];
	numberOfFrames = [paramInputFile numberOfFrames];
	paramFileId = paramInputFile;
	
	if ([paramInputFile noAlpha]==YES) printf("encodeStream: eluding alpha\n");
	
	f0[0] = 0;
	strcpy( f1, [paramInputFile getNextImageFilename]);
	if (( [paramInputFile moreFrames] == YES )) strcpy( f2, [paramInputFile getNextImageFilename]);
	morestuff = numberOfFrames;
	
	while( morestuff ) {
		[self loadAndDisplayImage: f1];
		
		if (onFrame==1 && ([image hadAlpha]==NO || [paramInputFile noAlpha]==YES) && ![self makingVideo] && ![self scaleable]) {
			[encoder sparseEncode: self];
//			[self writeLossless];
		} else {
			if (!strcmp( f0, f1 ) && strcmp( f1, f2) ) {
				[self writeHangFrame];
			} else {
				[encoder sparseEncode: self];
			}
		}

		onFrame++;
		strcpy( f0, f1 );
		strcpy( f1, f2 );
		if ([paramInputFile moreFrames] == YES) strcpy( f2, [paramInputFile getNextImageFilename]);
		morestuff--;
	}

	if (numberOfFrames != 1) {
		if ([image hadAlpha] && [paramInputFile noAlpha]==NO) {
			lastFrame = YES;
			[encoder sparseEncode: self];
		} else {
			[self writeLossless];		
		}
	}
	return self;
}

- write16Word:(word *)aWord to:(FILE *)stream
{
	byte	a, b;
	
	a = *aWord & 0xff;
	b = *aWord >> 8;

	fputc( a, stream );
	fputc( b, stream );

	return self;
}

- write32Word:( unsigned int *)aWord to:(FILE *)stream
{
	byte	a, b, c, d;
	
	a = *aWord & 0xff;
	b = (*aWord >> 8) & 0xff;
	c = (*aWord >> 16) & 0xff;
	d = (*aWord >> 24) & 0xff;

	fputc( a, stream );
	fputc( b, stream );
	fputc( c, stream );
	fputc( d, stream );
	return self;
}

-(int)sizeFile:(FILE *)ftosize;
{
	long int	fat, fend;
	fat = ftell(ftosize);
	fseek( ftosize, 0, SEEK_END );
	fend = ftell(ftosize);
	fseek( ftosize, fat, SEEK_SET);
	return (fend);
}

- convertPlanertoPacked
{
byte *iPlane[5], *newdata, *olddata;
int x,y,index, sample, pixelsWide, pixelsHigh;
TByteBitmapImageRep *newImage;

	pixelsWide = [image pixelsWide];
	pixelsHigh = [image pixelsHigh];
	
	printf("convertPlanertoPacked: converting\n");
	
		newImage = [[TByteBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
					pixelsWide: pixelsWide
					pixelsHigh: pixelsHigh
					bitsPerSample: 8
					samplesPerPixel: 4
					hasAlpha: YES
					isPlanar: NO
					colorSpaceName: NSCalibratedRGBColorSpace
					bytesPerRow: 0
					bitsPerPixel: 0];

	newdata = [newImage bitmapData];
	index = 0;

	if ([image isPlanar]) {
		[image getBitmapDataPlanes: iPlane];
		for(y=0;y<pixelsHigh;y++) {
		for(x=0;x<pixelsWide;x++) {
			newdata[index++] = iPlane[0][y*pixelsWide+x];
			newdata[index++] = iPlane[1][y*pixelsWide+x];
			newdata[index++] = iPlane[2][y*pixelsWide+x];
			if ([image hasAlpha]) {
				newdata[index++] = iPlane[3][y*pixelsWide+x];
			} else {
				newdata[index++] = 255;
			}
		}
		}
	} else {
		sample = 0;
		olddata = [image bitmapData];
		for(y=0;y<pixelsHigh;y++) {
		for(x=0;x<pixelsWide;x++) {
			newdata[index++] = olddata[sample++];
			newdata[index++] = olddata[sample++];
			newdata[index++] = olddata[sample++];
			if ([image hasAlpha]) {
				newdata[index++] = olddata[sample++];
			} else {
				newdata[index++] = 255;
			}
		}
		}
	}
	
	[image dealloc];
	image = newImage;

	return self;
}


- writeLossless
{
word direct;
unsigned int j;
char tempFile[MAXPATHLEN];
FILE *ftemp;
byte *buffer;
int res, mess;

	[self convertPlanertoPacked];
	if (!dataStuff) {
		[self initRoQPatterns];
		dataStuff=YES;
	}
	direct = RoQ_QUAD_JPEG;
	[self write16Word: &direct to: RoQFile];
	sprintf(tempFile, "%s.jpg",[paramFileId  roqTempFilename]);

	[image writeJFIF:tempFile quality: [paramFileId jpegQuality]];

	ftemp = fopen(tempFile, "rb");
	if (!ftemp) { fprintf(stderr, "Could not open temp file\n"); exit(1); }
	j = [self sizeFile: ftemp];
	printf("writeLossless: writing %d bytes to RoQ_QUAD_JPEG\n", j);
	[self write32Word: &j to: RoQFile];
	direct = 0;		// flags
	[self write16Word: &direct to: RoQFile];

	buffer = malloc( 16384 );
	do {
		res = fread( buffer, 1, 16384, ftemp);
		mess = fwrite( buffer, 1, res, RoQFile );
		if (res != mess) { fprintf(stderr, "Could not write to output stream\n"); exit(1); }
	} while ( res == 16384 );
	free( buffer );
	fclose(ftemp);
	[encoder setPreviousImage: tempFile from: image parent: self];
	remove( tempFile );
	fflush( RoQFile );

	return self;
}	

- initRoQFile:(const char *)RoQFilename
{
word i;
static int finit = 0;

	if (!finit) {
		finit++;
		printf("initRoQFile: %s\n", RoQFilename);
		RoQFile = fopen( RoQFilename, "w" );
//		chmod(RoQFilename, S_IREAD|S_IWRITE|S_ISUID|S_ISGID|0070|0007 );
		if (!RoQFile) {
			fprintf(stderr,"Unable to open output file %s.\n", RoQFilename);
			exit(1);
		}

		i = RoQ_ID;
		[self write16Word: &i to: RoQFile];

		i = 0xffff;
		[self write16Word: &i to: RoQFile];
		[self write16Word: &i to: RoQFile];

		i = 24;						// framerate
		[self write16Word: &i to: RoQFile];
		fflush( RoQFile );
	}
	strcpy( roqOutfile, RoQFilename );

	return self;
}

- initRoQPatterns
{
int j;
word direct;

	direct = RoQ_QUAD_INFO;
	[self write16Word: &direct to: RoQFile];

	j = 8;

	[self write32Word: &j to: RoQFile];
	printf("initRoQPatterns: outputting %d bytes to RoQ_INFO\n", j);
	direct = [image hadAlpha];
	if ([self paramNoAlpha] == YES) direct = 0;

	[self write16Word: &direct to: RoQFile];

	direct = [image pixelsWide];
	[self write16Word: &direct to: RoQFile];
	direct = [image pixelsHigh];
	[self write16Word: &direct to: RoQFile];
	direct = 8;
	[self write16Word: &direct to: RoQFile];
	direct = 4;
	[self write16Word: &direct to: RoQFile];

	fflush( RoQFile );

	return self;
}

- closeRoQFile
{
	fflush( RoQFile );
	printf("closeRoQFile: closing RoQ file\n");
	fclose( RoQFile );

	return self;
}

- writeHangFrame
{
int j;
word direct;
	printf("*******************************************************************\n");
	direct = RoQ_QUAD_HANG;
	[self write16Word: &direct to: RoQFile];
	j = 0;
	[self write32Word: &j to: RoQFile];
	direct = 0;
	[self write16Word: &direct to: RoQFile];
	return self;
}

- writeCodeBookToStream: (byte *)codebook size: (int)csize flags: (word)cflags
{
int j;
word direct;

	if (!csize) {
		printf("writeCodeBook: NO VQ DATA!!!!\n");
		return self;
	}
	
	direct = RoQ_QUAD_CODEBOOK;

	[self write16Word: &direct to: RoQFile];

	j = csize;

	[self write32Word: &j to: RoQFile];
	printf("writeCodeBook: outputting %d bytes to RoQ_QUAD_CODEBOOK\n", j);

	direct = cflags;
	[self write16Word: &direct to: RoQFile];

	fwrite( codebook, j, 1, RoQFile);

	fflush( RoQFile );

	return self;
}

- writeCodeBook: (byte *)codebook
{
	memcpy( codes, codebook, 4096 );
	return self;
}

- writeFrame:(quadcel *)pquad
{
word action, direct;
int	onCCC, onAction, i, j, code;
byte *cccList;
BOOL *use2, *use4;
int dx,dy,dxMean,dyMean,index2[256],index4[256], dimension;

	cccList = malloc( numQuadCels * 8);					// maximum length 
	use2 = malloc(256*sizeof(BOOL));
	use4 = malloc(256*sizeof(BOOL));

	for(i=0;i<256;i++) {
		use2[i] = NO;
		use4[i] = NO;
	}

	action = 0;
	j = onAction = 0;
	onCCC = 2;											// onAction going to go at zero

	dxMean = [encoder motMeanX];
	dyMean = [encoder motMeanY];

	if ([image hadAlpha]) dimension = 10; else dimension = 6;

	for (i=0; i<numQuadCels; i++) {
	if ( pquad[i].size && pquad[i].size < 16 ) {
		switch( pquad[i].status ) {
			case	SLD:
				use4[pquad[i].patten[0]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+0]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+1]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+2]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+3]] = YES;
				break;
			case	PAT:
				use4[pquad[i].patten[0]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+0]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+1]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+2]] = YES;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+3]] = YES;
				break;
			case	CCC:
				use2[pquad[i].patten[1]] = YES;
				use2[pquad[i].patten[2]] = YES;
				use2[pquad[i].patten[3]] = YES;
				use2[pquad[i].patten[4]] = YES;
		}
	}
	}

	if (!dataStuff) {
		dataStuff=YES;
		[self initRoQPatterns];
		if ([image hadAlpha]) i = 3584; else i = 2560;
		[self writeCodeBookToStream: codes size: i flags: 0];
		for(i=0;i<256;i++) {
			index2[i] = i;
			index4[i] = i;
		}
	} else {
		j = 0;
		for(i=0;i<256;i++) {
			if (use2[i]) {
				index2[i] = j;
				for(dx=0;dx<dimension;dx++) cccList[j*dimension+dx] = codes[i*dimension+dx];
				j++;
			}
		}
		code = j*dimension;
		direct = j;
		printf("writeFrame: really used %d 2x2 cels\n", j);
		j = 0;
		for(i=0;i<256;i++) {
			if (use4[i]) {
				index4[i] = j;
				for(dx=0;dx<4;dx++) cccList[j*4+code+dx] = index2[codes[i*4+(dimension*256)+dx]];
				j++;
			}
		}
		code += j*4;
		direct = (direct<<8) + j;
		printf("writeFrame: really used %d 4x4 cels\n", j);
		if ([image hadAlpha]) i = 3584; else i = 2560;
		if ( code == i || j == 256) {
			[self writeCodeBookToStream: codes size: i flags: 0];
		} else {
			[self writeCodeBookToStream: cccList size: code flags: direct];
		}
	}

	action = 0;
	j = onAction = 0;

	for (i=0; i<numQuadCels; i++) {
	if ( pquad[i].size && pquad[i].size < 16 ) {
		code = -1;
		switch( pquad[i].status ) {
			case	DEP:
				code = 3;
				break;
			case	SLD:
				code = 2;
				cccList[onCCC++] = index4[pquad[i].patten[0]];
				break;
			case	MOT:
				code = 0;
				break;
			case	FCC:
				code = 1;
				dx = ((pquad[i].domain >> 8  )) - 128 - dxMean + 8;
				dy = ((pquad[i].domain & 0xff)) - 128 - dyMean + 8;
				if (dx>15 || dx<0 || dy>15 || dy<0 ) {
					printf("writeFrame: FCC error %d,%d mean %d,%d at %d,%d,%d rmse %f\n", dx,dy, dxMean, dyMean,pquad[i].xat,pquad[i].yat,pquad[i].size, pquad[i].snr[FCC] );
					exit(1);
				}
				cccList[onCCC++] = (dx<<4)+dy;
				break;
			case	PAT:
				code = 2;
				cccList[onCCC++] = index4[pquad[i].patten[0]];
				break;
			case	CCC:
				code = 3;
				cccList[onCCC++] = index2[pquad[i].patten[1]];
				cccList[onCCC++] = index2[pquad[i].patten[2]];
				cccList[onCCC++] = index2[pquad[i].patten[3]];
				cccList[onCCC++] = index2[pquad[i].patten[4]];
				break;
			case	DEAD:
				fprintf(stderr,"dead cels in picture\n");
				break;
		}
		if (code == -1) {
			fprintf(stderr, "writeFrame: an error occurred writing the frame\n");
			exit(2);
		}

		action = (action<<2)|code;
		j++;
		if (j == 8) {
			j = 0;
			cccList[onAction+0] = (action & 0xff);
			cccList[onAction+1] = ((action >> 8) & 0xff);
			onAction = onCCC;
			onCCC += 2;
		}
	}
	}

	if (j) {
		action <<= ((8-j)*2);
		cccList[onAction+0] = (action & 0xff);
		cccList[onAction+1] = ((action >> 8) & 0xff);
	}

	direct = RoQ_QUAD_VQ;
	
	[self write16Word: &direct to: RoQFile];

	j = onCCC;
	[self write32Word: &j to: RoQFile];

	direct  = dyMean;
	direct &= 0xff;
	direct += (dxMean<<8);		// flags

	[self write16Word: &direct to: RoQFile];

	printf("writeFrame: outputting %d bytes to RoQ_QUAD_VQ\n", j);

	previousSize = j;
	
	fwrite( cccList, onCCC, 1, RoQFile );

	fflush( RoQFile );

	free( cccList );
	free( use2 );
	free( use4 );

	return self;
}

- writePuzzleFrame:(quadcel *)pquad
{
	return self;
}

- (TByteBitmapImageRep *)scaleImage: (TByteBitmapImageRep *)toscale
{
int newx, newy,x,y,s,linesize;
TByteBitmapImageRep *newImage;
unsigned char *i0, *i1;
int newv;

	newx = 288;
	
	if ([toscale pixelsHigh] == 160) {
		newy = 320;
	} else {
		newy = [toscale pixelsHigh];
	}
	newImage = [[TByteBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
				pixelsWide: newx
				pixelsHigh: newy
				bitsPerSample: 8
				samplesPerPixel: 4
				hasAlpha: YES
				isPlanar: NO
				colorSpaceName: NSCalibratedRGBColorSpace
				bytesPerRow: 0
				bitsPerPixel: 0];
				
	i0 = [toscale  bitmapData];
	i1 = [newImage bitmapData];
	linesize = [toscale pixelsWide]*4;
	
	for(y=0; y<newy; y++) {
	for(x=0; x<320; x++) {
		if (x>=16 && x<304) {
			for(s=0;s<4;s++) {
				if ([toscale pixelsHigh] == 160) {
					newv  = i0[(x*2+0)*4+(y>>1)*linesize+s];
					newv += i0[(x*2+1)*4+(y>>1)*linesize+s];
					newv += i0[(x*2+0)*4+((y>>1)+1)*linesize+s];
					newv += i0[(x*2+1)*4+((y>>1)+1)*linesize+s];
					newv = newv/4;
				} else {
					newv  = i0[(x*2+0)*4+y*linesize+s];
					newv += i0[(x*2+1)*4+y*linesize+s];
					newv = newv/2;	
				}
				i1[(x-16)*4+y*(288*4)+s] = newv;
			}
		}		
	}
	}
	return (newImage);
}

//
// load a frame, create a window (if neccesary) and display the frame
//
- loadAndDisplayImage: (const char *) filename
{
NSRect	cRect;
char *secondFilename,firstFilename[MAXPATHLEN+1];
id image1;
NSSize newSize;
// unsigned char *imageData;
// static BOOL cleared = NO;

	if (image) [image dealloc];

	printf("loadAndDisplayImage: %s\n", filename);

	strcpy( currentFile, filename );

	image = [TByteBitmapImageRep alloc];
	if (!(secondFilename= strchr(filename,'\n')))   {  //one filename, no compositing
		[image initFromFile: filename ];
	}
	else {
		strncpy(firstFilename,filename,secondFilename-filename);
		firstFilename[secondFilename-filename]='\0';
		secondFilename++;//point past the \n
		image1 = [[TByteBitmapImageRep alloc] initFromFile:firstFilename];
		[image initFromFile: secondFilename ];//result will be in here
		//image is the composite of those two...
		[self composite:image1 to:image];
	}
//
// wolf stuff
//
	newSize.width = 256;
	newSize.height = 256;
	
	[image setSize: newSize];
	
	if ([paramFileId output3DO] == YES) {
		image1 = [self scaleImage: image];
		[image dealloc];
		image = image1;
	}
	
	numQuadCels  = (([image pixelsWide] & 0xfff0)*([image pixelsHigh] & 0xfff0))/(MINSIZE*MINSIZE);
	numQuadCels += numQuadCels/4 + numQuadCels/16;

//	if ([paramFileId deltaFrames] == YES && cleared == NO && [image isPlanar] == NO) {
//		cleared = YES;
//		imageData = [image data];
//		memset( imageData, 0, [image pixelsWide]*[image pixelsHigh]*[image samplesPerPixel]);
//	}
	
	if (!quietMode) printf("loadAndDisplayImage: %dx%d\n", [image pixelsWide], [image pixelsHigh]);

	if (!quietMode) {
	if (!cWindow) {
	    cRect.origin.x = 8.0 * 48.0;
		cRect.origin.y = ([image pixelsHigh]+80);
    	cRect.size.width = [image pixelsWide];
    	cRect.size.height = [image pixelsHigh];
    	cWindow = [[NSWindow alloc] initWithContentRect:cRect styleMask:NSTitledWindowMask
        		backing:NSBackingStoreBuffered defer:NO];
    	cRect.origin.x = cRect.origin.y = 0.0;
//    	[[cWindow contentView] setClipping:NO];
	[cWindow setTitle: @"current frame"];
    	[cWindow makeKeyAndOrderFront: [cWindow contentView]];
  		[cWindow display];
	}
	

 	cRect = [[cWindow contentView] bounds];
	[[cWindow contentView] lockFocus];
	[image drawInRect: cRect];
	[[cWindow contentView] unlockFocus];
	[cWindow flushWindow];
	
	if (!eWindow) {
	    cRect.origin.x = 8.0 * 48.0 - ([image pixelsWide]>>1) - 4;
		cRect.origin.y = ([image pixelsHigh]+80);
    	cRect.size.width = [image pixelsWide] >> 1;
    	cRect.size.height = [image pixelsHigh] >> 1;
    	eWindow = [[NSWindow alloc] initWithContentRect:cRect styleMask:NSTitledWindowMask
        		backing:NSBackingStoreBuffered defer:NO];
    	cRect.origin.x = cRect.origin.y = 0.0;
//    	[[eWindow contentView] setClipping:NO];
    	[eWindow setTitle: @"cel error"];
    	[eWindow makeKeyAndOrderFront: [eWindow contentView]];
  		[eWindow display];
	}

	if (!sWindow) {
	    cRect.origin.x = 8.0 * 48.0 - ([image pixelsWide]>>1) - 4;
		cRect.origin.y = ([image pixelsHigh]+80) + (([image pixelsHigh]+48)>>1);
    	cRect.size.width = [image pixelsWide] >> 1;
    	cRect.size.height = [image pixelsHigh] >> 1;
    	sWindow = [[NSWindow alloc] initWithContentRect: cRect styleMask:NSTitledWindowMask
        		backing:NSBackingStoreBuffered defer:NO];
    	cRect.origin.x = cRect.origin.y = 0.0;
//    	[[eWindow contentView] setClipping:NO];
    	[sWindow setTitle: @"quadtree map"];
    	[sWindow makeKeyAndOrderFront: [sWindow contentView]];
  		[sWindow display];
	}

 	cRect = [[sWindow contentView] bounds];
	[[sWindow contentView] lockFocus];
	[image drawInRect: cRect];
	[[sWindow contentView] unlockFocus];
	[sWindow flushWindow];
	
 	cRect = [[eWindow contentView] bounds];
	[[eWindow contentView] lockFocus];
	[image drawInRect: cRect];
	[[eWindow contentView] unlockFocus];
	[eWindow flushWindow];
	
	if (!errImage) {
		errImage = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
				pixelsWide: 1
				pixelsHigh: 1
				bitsPerSample: 8
				samplesPerPixel: 3
				hasAlpha: NO
				isPlanar: NO
				colorSpaceName: NSCalibratedRGBColorSpace
				bytesPerRow: 0
				bitsPerPixel: 0];
	}

//	NSPing();
	}

	return self;
}


- markQuadx: (int)xat quady: (int)yat quads: (int)size error: (float)cerror type: (int)choice
{
NSRect	cRect;
byte *err;
static int ywasat = -1;
	if (!quietMode) {
		cRect.origin.x = (xat)>>1;
		cRect.origin.y = ([image pixelsHigh] - yat - size)>>1;
		cRect.size.width = cRect.size.height = (size)>>1;

		if (size < 1) {
		    [[sWindow contentView] lockFocus];
		    if (size == 8 && choice == 1) {
			PSsetgray(NSWhite);
		    } else if (size == 8 && choice == 3) {
			PSsetgray(NSLightGray);
		    } else if (size == 4) {
			PSsetgray(NSDarkGray);
		    } else if (size == 2) {
			PSsetgray(NSBlack);
		    }
		    NSFrameRectWithWidth(cRect,0.0);
		    [[sWindow contentView] unlockFocus];
		    if (!(ywasat & 31)) {
			[sWindow flushWindow];
		    }
		}
		
		err  = [errImage bitmapData];
		err[0] = err[1] = err[2] = 0;
		if ( cerror > 31 ) cerror = 31;
		if (choice & 1) err[0] = (int)cerror*8;
		if (choice & 2) err[1] = (int)cerror*8;
		if (choice & 4) err[2] = (int)cerror*8;

		[[eWindow contentView] lockFocus];
		[errImage drawInRect: cRect];
		[[eWindow contentView] unlockFocus];
		if (!(ywasat & 31)) {
		    [eWindow flushWindow];
		}
		ywasat++;
	}

	return self;
}

- (NSBitmapImageRep*)currentImage
{
	return	image;
}

- (id)errorWindow
{
	return eWindow;
}

- (id)scaleWindow
{
	return sWindow;
}
- (int)numberOfFrames {
	return numberOfFrames;
}

- composite:(NSBitmapImageRep *)source to: (NSBitmapImageRep *)destination
{
unsigned short value;
int x,y,bpp,inc,pixelsWide,pixelsHigh,yoff,pixel;
byte *iPlane[5], *dPlane[5];
	
	bpp = [source samplesPerPixel];
	pixelsWide = [source pixelsWide];
	pixelsHigh = [source pixelsHigh];
	
	if ([source isPlanar]) {
		[source getBitmapDataPlanes: iPlane];
		[destination getBitmapDataPlanes: dPlane];
		for(y=0;y<pixelsHigh;y++) {
			yoff = y*pixelsWide;
			for(x=0;x<pixelsWide;x++) {
				if ([destination hasAlpha]) {
					value = dPlane[3][yoff+x];
				} else {
					value = 255;
				}
				if (value == 0) {
					dPlane[0][yoff+x] = iPlane[0][yoff+x];
					dPlane[1][yoff+x] = iPlane[1][yoff+x];
					dPlane[2][yoff+x] = iPlane[2][yoff+x];
					dPlane[3][yoff+x] = iPlane[3][yoff+x];
				} else if (value != 255) {
					pixel = ((iPlane[0][yoff+x]*(255-value))/255) + ((dPlane[0][yoff+x]*value)/255);
					dPlane[0][yoff+x] = pixel;
					pixel = ((iPlane[1][yoff+x]*(255-value))/255) + ((dPlane[1][yoff+x]*value)/255);
					dPlane[1][yoff+x] = pixel;
					pixel = ((iPlane[2][yoff+x]*(255-value))/255) + ((dPlane[2][yoff+x]*value)/255);
					dPlane[2][yoff+x] = pixel;
					if ([destination hasAlpha]) {
						if (iPlane[3][yoff+x]>dPlane[3][yoff+x]) dPlane[3][yoff+x] = iPlane[3][yoff+x];
					}
				}
			}
		}
	} else {
		iPlane[0] = [source bitmapData];
		dPlane[0] = [destination bitmapData];
		for(y=0;y<pixelsHigh;y++) {
			yoff = y*pixelsWide*bpp;
			for(x=0;x<pixelsWide;x++) {
				inc = x*bpp;
				if ([destination hasAlpha]) {
					value = dPlane[0][yoff+inc+3];
				} else {
					value = 255;
				}
				if (value == 0) {
					dPlane[0][yoff+inc+0] = iPlane[0][yoff+inc+0];
					dPlane[0][yoff+inc+1] = iPlane[0][yoff+inc+1];
					dPlane[0][yoff+inc+2] = iPlane[0][yoff+inc+2];
					dPlane[0][yoff+inc+3] = iPlane[0][yoff+inc+3];
				} else if (value != 255) {	
					pixel = ((iPlane[0][yoff+inc+0]*(255-value))/255) + ((dPlane[0][yoff+inc+0]*value)/255);
					dPlane[0][yoff+inc+0] = pixel;
					pixel = ((iPlane[0][yoff+inc+1]*(255-value))/255) + ((dPlane[0][yoff+inc+1]*value)/255);
					dPlane[0][yoff+inc+1] = pixel;
					pixel = ((iPlane[0][yoff+inc+2]*(255-value))/255) + ((dPlane[0][yoff+inc+2]*value)/255);
					dPlane[0][yoff+inc+2] = pixel;
					if ([destination hasAlpha]) {
						if (iPlane[0][yoff+inc+3]>dPlane[0][yoff+inc+3]) dPlane[0][yoff+inc+3] = iPlane[0][yoff+inc+3];
					}
				}
			}
		}
	}

	return self;


}
@end

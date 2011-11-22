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

#include "roq.h"
#include "codec.h"

roq		*theRoQ;				// current roq file

roq::roq( void )
{
	image = 0;
	quietMode = false;
	encoder = 0;
	previousSize = 0;
	lastFrame = false;
	dataStuff=false;
}

roq::~roq( void )
{
	if (image) delete image;
	if (encoder) delete encoder;
	return;
}

void roq::EncodeQuietly( bool which )
{
	quietMode = which;
}

bool roq::IsQuiet( void )
{
	return quietMode;
}

bool roq::IsLastFrame( void )
{
	return lastFrame;
}

bool roq::Scaleable( void )
{
	return paramFile->IsScaleable();
}

bool roq::ParamNoAlpha( void )
{
	return paramFile->NoAlpha();
}

bool roq::MakingVideo( void )
{
	return true;	//paramFile->timecode];
}

bool roq::SearchType( void )
{
	return	paramFile->SearchType();
}

bool roq::HasSound( void )
{
	return	paramFile->HasSound();
}

int roq::PreviousFrameSize( void )
{
	return	previousSize;
}

int roq::FirstFrameSize( void )
{
	return paramFile->FirstFrameSize();
}

int roq::NormalFrameSize( void )
{
	return	paramFile->NormalFrameSize();
}

const char * roq::CurrentFilename( void )
{
	return currentFile.c_str();
}

void roq::EncodeStream( const char *paramInputFile )
{
	int		onFrame;
	idStr	f0, f1, f2;
	int		morestuff;

	onFrame = 1;
	
	encoder = new codec;
	paramFile = new roqParam;
	paramFile->numInputFiles = 0;
	
	paramFile->InitFromFile( paramInputFile );

	if (!paramFile->NumberOfFrames()) {
		return;
	}
	
	InitRoQFile( paramFile->outputFilename);

	numberOfFrames = paramFile->NumberOfFrames();

	if (paramFile->NoAlpha()==true) common->Printf("encodeStream: eluding alpha\n");
	
	f0 = "";
	f1 = paramFile->GetNextImageFilename();
	if (( paramFile->MoreFrames() == true )) {
		f2 = paramFile->GetNextImageFilename();
	}
	morestuff = numberOfFrames;
	
	while( morestuff ) {
		LoadAndDisplayImage( f1 );
		
		if (onFrame==1) {
			encoder->SparseEncode();
//			WriteLossless();
		} else {
			if (!strcmp( f0, f1 ) && strcmp( f1, f2) ) {
				WriteHangFrame();
			} else {
				encoder->SparseEncode();
			}
		}

		onFrame++;
		f0 = f1;
		f1 = f2;
		if (paramFile->MoreFrames() == true) {
			f2 = paramFile->GetNextImageFilename();
		}
		morestuff--;
		session->UpdateScreen();
	}

//	if (numberOfFrames != 1) {
//		if (image->hasAlpha() && paramFile->NoAlpha()==false) {
//			lastFrame = true;
//			encoder->SparseEncode();
//		} else {
//			WriteLossless();
//		}
//	}
	CloseRoQFile();
}

void roq::Write16Word( word *aWord, idFile *stream )
{
	byte	a, b;
	
	a = *aWord & 0xff;
	b = *aWord >> 8;

	stream->Write( &a, 1 );
	stream->Write( &b, 1 );
}

void roq::Write32Word( unsigned int *aWord, idFile *stream )
{
	byte	a, b, c, d;
	
	a = *aWord & 0xff;
	b = (*aWord >> 8) & 0xff;
	c = (*aWord >> 16) & 0xff;
	d = (*aWord >> 24) & 0xff;

	stream->Write( &a, 1 );
	stream->Write( &b, 1 );
	stream->Write( &c, 1 );
	stream->Write( &d, 1 );
}

int roq::SizeFile( idFile *ftosize )
{
	return ftosize->Length();
}

/* Expanded data destination object for stdio output */

typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */

  byte* outfile;		/* target stream */
  int	size;
} my_destination_mgr;

typedef my_destination_mgr * my_dest_ptr;


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

void roq::JPEGInitDestination (j_compress_ptr cinfo) {
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

  dest->pub.next_output_byte = dest->outfile;
  dest->pub.free_in_buffer = dest->size;
}


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return true
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

boolean roq::JPEGEmptyOutputBuffer (j_compress_ptr cinfo) {
  return true;
}


/*
 * Compression initialization.
 * Before calling this, all parameters and a data destination must be set up.
 *
 * We require a write_all_tables parameter as a failsafe check when writing
 * multiple datastreams from the same compression object.  Since prior runs
 * will have left all the tables marked sent_table=true, a subsequent run
 * would emit an abbreviated stream (no tables) by default.  This may be what
 * is wanted, but for safety's sake it should not be the default behavior:
 * programmers should have to make a deliberate choice to emit abbreviated
 * images.  Therefore the documentation and examples should encourage people
 * to pass write_all_tables=true; then it will take active thought to do the
 * wrong thing.
 */

void roq::JPEGStartCompress (j_compress_ptr cinfo, bool write_all_tables) {
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  if (write_all_tables)
    jpeg_suppress_tables(cinfo, FALSE);	/* mark all tables to be written */

  /* (Re)initialize error mgr and destination modules */
  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);
  (*cinfo->dest->init_destination) (cinfo);
  /* Perform master selection of active modules */
  jinit_compress_master(cinfo);
  /* Set up for the first pass */
  (*cinfo->master->prepare_for_pass) (cinfo);
  /* Ready for application to drive first pass through jpeg_write_scanlines
   * or jpeg_write_raw_data.
   */
  cinfo->next_scanline = 0;
  cinfo->global_state = (cinfo->raw_data_in ? CSTATE_RAW_OK : CSTATE_SCANNING);
}


/*
 * Write some scanlines of data to the JPEG compressor.
 *
 * The return value will be the number of lines actually written.
 * This should be less than the supplied num_lines only in case that
 * the data destination module has requested suspension of the compressor,
 * or if more than image_height scanlines are passed in.
 *
 * Note: we warn about excess calls to jpeg_write_scanlines() since
 * this likely signals an application programmer error.  However,
 * excess scanlines passed in the last valid call are *silently* ignored,
 * so that the application need not adjust num_lines for end-of-image
 * when using a multiple-scanline buffer.
 */

JDIMENSION roq::JPEGWriteScanlines (j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines) {
  JDIMENSION row_ctr, rows_left;

  if (cinfo->global_state != CSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline >= cinfo->image_height)
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);

  /* Call progress monitor hook if present */
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long) cinfo->next_scanline;
    cinfo->progress->pass_limit = (long) cinfo->image_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr) cinfo);
  }

  /* Give master control module another chance if this is first call to
   * jpeg_write_scanlines.  This lets output of the frame/scan headers be
   * delayed so that application can write COM, etc, markers between
   * jpeg_start_compress and jpeg_write_scanlines.
   */
  if (cinfo->master->call_pass_startup)
    (*cinfo->master->pass_startup) (cinfo);

  /* Ignore any extra scanlines at bottom of image. */
  rows_left = cinfo->image_height - cinfo->next_scanline;
  if (num_lines > rows_left)
    num_lines = rows_left;

  row_ctr = 0;
  (*cinfo->main->process_data) (cinfo, scanlines, &row_ctr, num_lines);
  cinfo->next_scanline += row_ctr;
  return row_ctr;
}

/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

static int hackSize;

void roq::JPEGTermDestination (j_compress_ptr cinfo) {
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
  size_t datacount = dest->size - dest->pub.free_in_buffer;
  hackSize = datacount;
}


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

void roq::JPEGDest (j_compress_ptr cinfo, byte* outfile, int size) {
  my_dest_ptr dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_stdio_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL) {	/* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(my_destination_mgr));
  }

  dest = (my_dest_ptr) cinfo->dest;
  dest->pub.init_destination = JPEGInitDestination;
  dest->pub.empty_output_buffer = JPEGEmptyOutputBuffer;
  dest->pub.term_destination = JPEGTermDestination;
  dest->outfile = outfile;
  dest->size = size;
}

void roq::WriteLossless( void ) {

	word direct;
	uint directdw;

	if (!dataStuff) {
		InitRoQPatterns();
		dataStuff=true;
	}
	direct = RoQ_QUAD_JPEG;
	Write16Word( &direct, RoQFile);

	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;
	/* More stuff */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */
	byte *out;

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/
	out = (byte *)Mem_Alloc(image->pixelsWide()*image->pixelsHigh()*4);
	JPEGDest(&cinfo, out, image->pixelsWide()*image->pixelsHigh()*4);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = image->pixelsWide(); 	/* image width and height, in pixels */
	cinfo.image_height = image->pixelsHigh();
	cinfo.input_components = 4;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, paramFile->JpegQuality(), true /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* true ensures that we will write a complete interchange-JPEG file.
	* Pass true unless you are very sure of what you're doing.
	*/
	JPEGStartCompress(&cinfo, true);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = image->pixelsWide() * 4;	/* JSAMPLEs per row in image_buffer */

	byte *pixbuf = image->bitmapData();
	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		row_pointer[0] = &pixbuf[((cinfo.image_height-1)*row_stride)-cinfo.next_scanline * row_stride];
		(void) JPEGWriteScanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */

	directdw = hackSize;
	common->Printf("writeLossless: writing %d bytes to RoQ_QUAD_JPEG\n", hackSize);
	Write32Word( &directdw, RoQFile );
	direct = 0;		// flags
	Write16Word( &direct, RoQFile );

	RoQFile->Write( out, hackSize );
	Mem_Free(out);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */
	encoder->SetPreviousImage( "first frame", image );
}

void roq::InitRoQFile( const char *RoQFilename )
{
	word i;
	static int finit = 0;

	if (!finit) {
		finit++;
		common->Printf("initRoQFile: %s\n", RoQFilename);
		RoQFile = fileSystem->OpenFileWrite( RoQFilename );
//		chmod(RoQFilename, S_IREAD|S_IWRITE|S_ISUID|S_ISGID|0070|0007 );
		if ( !RoQFile ) {
			common->Error("Unable to open output file %s.\n", RoQFilename);
		}

		i = RoQ_ID;
		Write16Word( &i, RoQFile );

		i = 0xffff;
		Write16Word( &i, RoQFile );
		Write16Word( &i, RoQFile );

		// to retain exact file format write out 32 for new roq's
		// on loading this will be noted and converted to 1000 / 30
		// as with any new sound dump avi demos we need to playback
		// at the speed the sound engine dumps the audio
		i = 30;						// framerate
		Write16Word( &i, RoQFile );
	}
	roqOutfile = RoQFilename;
}

void roq::InitRoQPatterns( void )
{
uint j;
word direct;

	direct = RoQ_QUAD_INFO;
	Write16Word( &direct, RoQFile );

	j = 8;

	Write32Word( &j, RoQFile );
	common->Printf("initRoQPatterns: outputting %d bytes to RoQ_INFO\n", j);
	direct = image->hasAlpha();
	if (ParamNoAlpha() == true) direct = 0;

	Write16Word( &direct, RoQFile );

	direct = image->pixelsWide();
	Write16Word( &direct, RoQFile );
	direct = image->pixelsHigh();
	Write16Word( &direct, RoQFile );
	direct = 8;
	Write16Word( &direct, RoQFile );
	direct = 4;
	Write16Word( &direct, RoQFile );
}

void roq::CloseRoQFile( void )
{
	common->Printf("closeRoQFile: closing RoQ file\n");
	fileSystem->CloseFile( RoQFile );
}

void roq::WriteHangFrame( void )
{
uint j;
word direct;
	common->Printf("*******************************************************************\n");
	direct = RoQ_QUAD_HANG;
	Write16Word( &direct, RoQFile);
	j = 0;
	Write32Word( &j, RoQFile);
	direct = 0;
	Write16Word( &direct, RoQFile);
}

void roq::WriteCodeBookToStream( byte *codebook, int csize, word cflags )
{
uint j;
word direct;

	if (!csize) {
		common->Printf("writeCodeBook: false VQ DATA!!!!\n");
		return;
	}
	
	direct = RoQ_QUAD_CODEBOOK;

	Write16Word( &direct, RoQFile);

	j = csize;

	Write32Word( &j, RoQFile);
	common->Printf("writeCodeBook: outputting %d bytes to RoQ_QUAD_CODEBOOK\n", j);

	direct = cflags;
	Write16Word( &direct, RoQFile);

	RoQFile->Write( codebook, j );
}

void roq::WriteCodeBook( byte *codebook )
{
	memcpy( codes, codebook, 4096 );
}

void roq::WriteFrame( quadcel *pquad )
{
word action, direct;
int	onCCC, onAction, i, code;
uint j;
byte *cccList;
bool *use2, *use4;
int dx,dy,dxMean,dyMean,index2[256],index4[256], dimension;

	cccList = (byte *)Mem_Alloc( numQuadCels * 8);					// maximum length 
	use2 = (bool *)Mem_Alloc(256*sizeof(bool));
	use4 = (bool *)Mem_Alloc(256*sizeof(bool));

	for(i=0;i<256;i++) {
		use2[i] = false;
		use4[i] = false;
	}

	action = 0;
	j = onAction = 0;
	onCCC = 2;											// onAction going to go at zero

	dxMean = encoder->MotMeanX();
	dyMean = encoder->MotMeanY();

	if (image->hasAlpha()) dimension = 10; else dimension = 6;

	for (i=0; i<numQuadCels; i++) {
	if ( pquad[i].size && pquad[i].size < 16 ) {
		switch( pquad[i].status ) {
			case	SLD:
				use4[pquad[i].patten[0]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+0]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+1]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+2]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+3]] = true;
				break;
			case	PAT:
				use4[pquad[i].patten[0]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+0]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+1]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+2]] = true;
				use2[codes[dimension*256+(pquad[i].patten[0]*4)+3]] = true;
				break;
			case	CCC:
				use2[pquad[i].patten[1]] = true;
				use2[pquad[i].patten[2]] = true;
				use2[pquad[i].patten[3]] = true;
				use2[pquad[i].patten[4]] = true;
		}
	}
	}

	if (!dataStuff) {
		dataStuff=true;
		InitRoQPatterns();
		if (image->hasAlpha()) i = 3584; else i = 2560;
		WriteCodeBookToStream( codes, i, 0 );
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
		common->Printf("writeFrame: really used %d 2x2 cels\n", j);
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
		common->Printf("writeFrame: really used %d 4x4 cels\n", j);
		if (image->hasAlpha()) i = 3584; else i = 2560;
		if ( code == i || j == 256) {
			WriteCodeBookToStream( codes, i, 0 );
		} else {
			WriteCodeBookToStream( cccList, code, direct );
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
					common->Error("writeFrame: FCC error %d,%d mean %d,%d at %d,%d,%d rmse %f\n", dx,dy, dxMean, dyMean,pquad[i].xat,pquad[i].yat,pquad[i].size, pquad[i].snr[FCC] );
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
				common->Error("dead cels in picture\n");
				break;
		}
		if (code == -1) {
			common->Error( "writeFrame: an error occurred writing the frame\n");
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
	
	Write16Word( &direct, RoQFile);

	j = onCCC;
	Write32Word( &j, RoQFile);

	direct  = dyMean;
	direct &= 0xff;
	direct += (dxMean<<8);		// flags

	Write16Word( &direct, RoQFile);

	common->Printf("writeFrame: outputting %d bytes to RoQ_QUAD_VQ\n", j);

	previousSize = j;
	
	RoQFile->Write( cccList, onCCC );

	Mem_Free( cccList );
	Mem_Free( use2 );
	Mem_Free( use4 );
}

//
// load a frame, create a window (if neccesary) and display the frame
//
void roq::LoadAndDisplayImage( const char * filename )
{
	if (image) delete image;

	common->Printf("loadAndDisplayImage: %s\n", filename);

	currentFile = filename;

	image = new NSBitmapImageRep( filename );
	
	numQuadCels  = ((image->pixelsWide() & 0xfff0)*(image->pixelsHigh() & 0xfff0))/(MINSIZE*MINSIZE);
	numQuadCels += numQuadCels/4 + numQuadCels/16;

//	if (paramFile->deltaFrames] == true && cleared == false && [image isPlanar] == false) {
//		cleared = true;
//		imageData = [image data];
//		memset( imageData, 0, image->pixelsWide()*image->pixelsHigh()*[image samplesPerPixel]);
//	}
	
	if (!quietMode) common->Printf("loadAndDisplayImage: %dx%d\n", image->pixelsWide(), image->pixelsHigh());
}

void roq::MarkQuadx( int xat, int yat, int size, float cerror, int choice ) {
}

NSBitmapImageRep* roq::CurrentImage( void )
{
	return	image;
}

int roq::NumberOfFrames( void ) {
	return numberOfFrames;
}

void RoQFileEncode_f( const idCmdArgs &args ) {
	if ( args.Argc() != 2 ) {
		common->Printf( "Usage: roq <paramfile>\n" );
		return;
	}
	theRoQ = new roq;
	int		startMsec = Sys_Milliseconds();
	theRoQ->EncodeStream( args.Argv( 1 ) );
	int		stopMsec = Sys_Milliseconds();
	common->Printf( "total encoding time: %i second\n", ( stopMsec - startMsec ) / 1000 );

}

#ifndef AUDIO_CORE_H
#define AUDIO_CORE_H
/*
 * Copyright by 4Front Technologies 1993-2004
 *
 * All rights reserved.
 */

/*
 * IMPORTANT NOTICE!
 *
 * This file contains internal structures used by Open Sound Systems.
 * They will change without any notice between OSS versions. Care must be taken
 * to make sure any software using this header gets properly re-compiled before
 * use.
 *
 * 4Front Technologies (or anybody else) takes no responsibility of damages
 * caused by use of this file.
 */

/* Max number of audio channels currently supported */
#define OSS_MAX_AUDIO_CHANNELS	8

struct audio_operations;

typedef struct
{
  int fmt, speed, channels;
  int convert;
}
sample_parms;

typedef struct audio_operations audio_operations, adev_t, *adev_p;
typedef struct dma_buffparms dma_buffparms, *dmap_p;
typedef int (*cnv_func_t) (adev_p adev, dmap_p dmap, void **srcp, int *srcl,
			   void **tgtp, sample_parms * source,
			   sample_parms * target);

struct dma_buffparms
{
/*
 * Static fields (not to be cleared during open)
 */
#ifndef CONFIGURE_C
  oss_mutex mutex;
#endif
  sound_os_info *osp;
  char *raw_buf;
  oss_native_ulong raw_buf_phys;
  int buffsize;
  unsigned char *tmpbuf1, *tmpbuf2;
  int dma;			/* DMA channel */
  void *driver_use_ptr;
  long driver_use_value;
  /* Interrupt callback stuff */
  void (*audio_callback) (int dev, int parm);
  int callback_parm;

#ifdef OS_DMA_PARMS
    OS_DMA_PARMS
#endif
/*
 * Dynamic fields (will be zeroed during open)
 * Don't add anything before flags.
 */
  void *srcstate[OSS_MAX_AUDIO_CHANNELS];
  oss_native_ulong flags;
#define DMAP_NOTIMEOUT	0x00000001
#define DMAP_POST		0x00000002
#define DMAP_PREPARED	0x00000004
#define DMAP_FRAGFIXED	0x00000008	/* Fragment size fixed */
#define DMAP_STARTED	0x00000010
#define DMAP_COOKED		0x00000020
#define DMAP_ACTIVE     0x00000040	/* ISA DMA is running */
  int dma_mode;			/* DMODE_INPUT, DMODE_OUTPUT or DMODE_NONE */
#define DMODE_NONE		0
#define DMODE_OUTPUT		PCM_ENABLE_OUTPUT
#define DMODE_INPUT		PCM_ENABLE_INPUT

  /*
   * Queue parameters.
   */
  int nbufs;
  int frag_used;
  int fragment_size;
  int bytes_in_use;
  int data_rate;		/* Bytes/second */
  int frame_size;
  int fragsize_rq;
  int low_water;
  volatile oss_native_ulonglong byte_counter;
  volatile oss_native_ulonglong user_counter;
  int write_count;
  int interrupt_count;
  int fragment_counter;
  int expand_factor;

  int mapping_flags;
#define			DMA_MAP_MAPPED		0x00000001
  char neutral_byte;

#ifdef SPARCAUDIO_EMU
#define EOFLIST_SIZE 16
  void *devaudio_sigproc;	/* A pref to which send a SIGPOLL signal */
  int devaudio_sigmask;
  int eof_head, eof_tail;
  int eof_list[EOFLIST_SIZE], eof_counts[EOFLIST_SIZE];
#endif
  int error;
  int play_underruns, rec_overruns;
  int underrun_flag;
  int play_error, num_play_errors;
  int rec_error, num_rec_errors;

  unsigned char *leftover_buf;
  int leftover_bytes;
  int tmpbuf_len, tmpbuf_ptr;
  cnv_func_t convert_func;
  unsigned int convert_mode;
  struct audio_buffer *(*user_import) (struct audio_operations * adev,
				       struct dma_buffparms * dmap,
				       sample_parms * parms,
				       unsigned char *cbuf, int len);
  int (*user_export) (struct audio_operations * adev,
		      struct dma_buffparms * dmap, sample_parms * parms,
		      struct audio_buffer * buf, unsigned char *cbuf,
		      int maxbytes);
  struct audio_buffer *(*device_read) (struct audio_operations * adev,
				       struct dma_buffparms * dmap,
				       sample_parms * parms,
				       unsigned char *cbuf, int len);
  int (*device_write) (struct audio_operations * adev,
		       struct dma_buffparms * dmap,
		       void *frombuf, void *tobuf,
		       int maxspace, int *fromlen, int *tolen);
};
extern int dmap_get_qlen (dma_buffparms * dmap);
extern int dmap_get_qhead (dma_buffparms * dmap);
extern int dmap_get_qtail (dma_buffparms * dmap);

struct audio_driver
{
  int (*open) (int dev, int mode, int open_flags);
  void (*close) (int dev, int mode);
  void (*output_block) (int dev, oss_native_ulong buf,
			int count, int fragsize, int intrflag);
  void (*start_input) (int dev, oss_native_ulong buf,
		       int count, int fragsize, int intrflag);
  int (*ioctl) (int dev, unsigned int cmd, ioctl_arg arg);
  int (*prepare_for_input) (int dev, int bufsize, int nbufs);
  int (*prepare_for_output) (int dev, int bufsize, int nbufs);
  void (*halt_io) (int dev);
  int (*local_qlen) (int dev);
  int (*copy_user) (int dev, char *localbuf, int localoffs,
		    WR_BUF_CONST snd_rw_buf * userbuf, int useroffs,
		    int *len, int max_space);
  void (*halt_input) (int dev);
  void (*halt_output) (int dev);
  void (*trigger) (int dev, int bits);
  int (*set_speed) (int dev, int speed);
  unsigned int (*set_bits) (int dev, unsigned int bits);
  short (*set_channels) (int dev, short channels);
  void (*postprocess_write) (int dev);	/* Device spesific postprocessing for written data */
  void (*preprocess_read) (int dev);	/* Device spesific preprocessing for read data */
  /* Timeout handlers for input and output */
  int (*check_input) (int dev);
  int (*check_output) (int dev);

  int (*alloc_buffer) (int dev, struct dma_buffparms * dmap, int direction);
  int (*free_buffer) (int dev, struct dma_buffparms * dmap, int direction);
  void (*lock_buffer) (int dev, int direction);
  void *dummy;
  int (*get_buffer_pointer) (int dev, struct dma_buffparms * dmap,
			     int direction);
  int (*calibrate_speed) (int dev, int nominal_rate, int true_rate);
#define SYNC_PREPARE	1
#define SYNC_TRIGGER	2
  int (*sync_control) (int dev, int event, int mode);
  void (*prepare_to_stop) (int dev);
  int (*get_input_pointer) (int dev, struct dma_buffparms * dmap,
			    int direction);
  int (*get_output_pointer) (int dev, struct dma_buffparms * dmap,
			     int direction);
  int (*bind) (int dev, unsigned int cmd, ioctl_arg arg);
  void (*setup_fragments) (int dev, dmap_p dmap, int direction);
};

struct audio_operations
{
  char name[128];
  char handle[32];
  int dev;			/* Device's own index */
  int enabled;
  struct audio_operations *next;	/* Link to the next "shadow" device */
  int flags;
  int open_flags;
  int caps;
  int magic;			/* Secret low level driver ID */
#define NOTHING_SPECIAL 	0x00
#define NEEDS_RESTART		0x01
#define DMA_AUTOMODE		0x02
#define DMA_DUPLEX		0x04
#define DMA_COLD		0x08
#define DMA_UNUSED1		0x10
#define DMA_UNUSED2		0x40
#define DMA_UNUSED3		0x80
#define DMA_ISA			0x100	/* ISA DMA buffer placement restrictions */
#define DMA_VIRTUAL		0x400	/* Virtual audio device */
#define DMA_OPENED		0x800	/* Will be set when the device is open */
#define DMA_NOCONVERT		0x1000	/* No implicit format conversions */
#define DMA_DUALBUF		0x2000	/* Alloc separate bufs for rec and play */
#define DMA_USEPHYSADDR		0x4000	/* Use raw_buf_phys when mmap()ing */
#define DMA_DISABLED		0x8000
#define DMA_NOINPUT		0x10000
#define DMA_NOOUTPUT		0x20000
#define DMA_FIXEDRATE		0x40000	/* Fixed sampling rate */
#define DMA_16BITONLY		0x80000	/* Only 16 bit support */
#define DMA_STEREOONLY		0x100000	/* Only stereo (requires 16BITONLY) */
#define DMA_HUSHOUTPUT		0x200000	/* Do not permit use with O_WRONLY */
#define DMA_SHADOW		0x400000	/* "shadow" device */
#define DMA_ISABUS		0x800000	/* ISA device */
#define DMA_NODMA		0x1000000	/* For ISA devices only */
#define DMA_8BITONLY		0x2000000	/* Only 8 bits */
#define DMA_32BITONLY		0x4000000	/* Only 24 or 32 bits */
#define DMA_NOSOFTOSS		0x8000000	/* Don't install SoftOSS automatically for this device */
#define DMA_NOSRC		0x10000000	/* Don't do any kind of SRC */
#define DMA_SPECIAL		0x20000000	/* Multich or otherwise special dev */
#define DMA_NOMMAP		0x40000000	/* No MMAP capability */
#define DMA_SOFTOSS_DISABLE	0x80000000	/* Not compatible with SoftOSS  */


  /*
   * Sampling parameters
   */

  sample_parms user_parms, hw_parms;
  int iformat_mask, oformat_mask;	/* Bitmasks for supported audio formats */
  int min_rate, max_rate;	/* Sampling rate limits */
  int min_channels, max_channels;
  int xformat_mask;		/* Format mask for current open mode */
  int binding;
  void *devc;			/* Driver specific info */
  struct audio_driver *d;
  void *portc, *portc_play, *portc_record;	/* Driver specific info */
  struct dma_buffparms *dmap_in, *dmap_out;
  int mixer_dev;
  int open_mode;
  int go;
  int enable_bits;
  int parent_dev;		/* 0 -> no parent, 1 to n -> parent=parent_dev+1 */
  int max_block;		/* Maximum fragment size to be accepted */
  int min_block;		/* Minimum fragment size */
  int max_intrate;		/* Another form of min_block */
  int fixed_rate;
  int vmix_flags;		/* special flags sent to virtual mixer */
#define VMIX_MULTIFRAG	0x00000001	/* More than 2 fragments required (causes longer latencies) */
  int src_rate;
  int src_ratio;
  pid_t pid;
  char cmd[16];
  sound_os_info *osp;
  int setfragment_warned;
  int riff_warned;
  int redirect_in, redirect_out;
  int dmask;			/* Open dmaps */
#define DMASK_OUT		0x01
#define DMASK_IN		0x02
  int nonblock;
  int forced_nonblock;
  int ossd_registered;
  int sync_flags;
#define SYNC_MASTER		0x01
#define SYNC_SLAVE		0x02
  int sync_group;
  int sync_mode;
  struct audio_operations *sync_next;	/* Next device in sync group */

  int rate_source;
#define MAX_SAMPLE_RATES	20 /* Cannot be changed (see soundcard.h) */
  int nrates, rates[MAX_SAMPLE_RATES];

#ifndef CONFIGURE_C
  oss_mutex mutex;
#endif

  int card_number;
  int port_number;
  int real_dev;

  int cooked_enable;
  int timeout_count;

  void (*outputintr) (int dev, int xx);
  void (*inputintr) (int dev);
};

typedef struct oss_card_desc
{
  char shortname[16];
  char longname[128];
} oss_card_desc_t, *oss_card_desc_p;

#define UNIT_EXPAND		1024

extern struct audio_operations **audio_devs;
extern int num_audiodevs;
extern oss_card_desc_p *oss_cardlist;
extern const char *oss_version_string;
extern const char *oss_checksum;
#endif

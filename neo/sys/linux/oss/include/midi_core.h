#ifndef MIDI_CORE_H
#define MIDI_CORE_H
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

struct midi_input_info
{				/* MIDI input scanner variables */
#define MI_MAX	32
  int m_busy;
  unsigned char m_buf[MI_MAX];
  unsigned char m_prev_status;	/* For running status */
  int m_ptr;
#define MST_INIT			0
#define MST_DATA			1
#define MST_SYSEX			2
  int m_state;
  int m_left;
};

typedef struct midi_operations
{
  struct midi_info info;
  struct synth_operations *converter;
  struct midi_input_info in_info;
  int (*open) (int dev, int mode,
	       void (*inputintr) (int dev, unsigned char data),
	       void (*outputintr) (int dev));
  void (*close) (int dev);
  int (*ioctl) (int dev, unsigned int cmd, ioctl_arg arg);
  int (*outputc) (int dev, unsigned char data);
  int (*start_read) (int dev);
  int (*end_read) (int dev);
  void (*kick) (int dev);
  int (*command) (int dev, unsigned char *data);
  int (*buffer_status) (int dev);
  int (*prefix_cmd) (int dev, unsigned char status);
  void (*input_callback) (int dev, unsigned char midich);
  struct coproc_operations *coproc;
  void *devc;
  sound_os_info *osp;
  int card_number;
#ifndef CONFIGURE_C
  oss_mutex mutex;
#endif
  unsigned long flags;
#define MFLAG_NOSEQUENCER		0x00000001 /* Not to be used by the sequencer driver */
} mididev_t, *mididev_p;

extern struct midi_operations **midi_devs;
extern int num_mididevs;
#endif

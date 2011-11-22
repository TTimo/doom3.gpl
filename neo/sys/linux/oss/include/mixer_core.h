#ifndef MIXER_CORE_H
#define MIXER_CORE_H
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
typedef int (*mixer_ext_fn) (int dev, int ctrl, unsigned int cmd, int value);
typedef int (*mixer_ext_init_fn) (int dev);

typedef struct
{
  oss_mixext ext;
  mixer_ext_fn handler;
  oss_mixer_enuminfo *enum_info;
}
oss_mixext_desc;

struct mixer_operations
{
  char id[16];
  char name[64];
  int (*ioctl) (int dev, int audiodev, unsigned int cmd, ioctl_arg arg);

  void *devc;
  void *hw_devc;
  int modify_counter;

  /* Mixer extension interface */
  int nr_ext;
  int max_ext;
  int nr_extra_ext;
  int timestamp;
  oss_mixext_desc *extensions;
  mixer_ext_init_fn ext_init_fn;
  int ignore_mask;		/* Controls ignored by mixer ext API */
  int card_number;
  int enabled;
};

typedef struct mixer_operations mixdev_t, *mixdev_p;

extern struct mixer_operations **mixer_devs;
extern int num_mixers;
extern void touch_mixer (int dev);
extern int oss_mixer_ext (int orig_dev, unsigned int cmd, ioctl_arg arg);
extern int mixer_ext_set_enum (oss_mixer_enuminfo * ent);

#endif

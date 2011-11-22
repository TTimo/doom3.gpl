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

#define IDD_DIALOG_AF                            100
#define IDD_DIALOG_AF_NAME                       101
#define IDD_DIALOG_AF_VIEW                       102
#define IDD_DIALOG_AF_PROPERTIES                 103
#define IDD_DIALOG_AF_BODY                       104
#define IDD_DIALOG_AF_CONSTRAINT                 105
#define IDD_DIALOG_AF_CONSTRAINT_FIXED           106
#define IDD_DIALOG_AF_CONSTRAINT_BALLANDSOCKET   107
#define IDD_DIALOG_AF_CONSTRAINT_UNIVERSAL       108
#define IDD_DIALOG_AF_CONSTRAINT_HINGE           109
#define IDD_DIALOG_AF_CONSTRAINT_SLIDER          110
#define IDD_DIALOG_AF_CONSTRAINT_SPRING          111
#define IDI_ICON2                                112

#define IDC_DIALOG_AF_TAB_MODE                   200
#define IDC_DIALOG_AF_START                      201
#define IDC_COMBO_AF                             202
#define IDC_BUTTON_AF_NEW                        203
#define IDC_BUTTON_AF_DELETE                     204
#define IDC_BUTTON_AF_SPAWN                      205
#define IDC_BUTTON_AF_TPOSE                      206
#define IDC_BUTTON_AF_KILL                       207
#define IDC_BUTTON_AF_SAVE                       208
#define IDC_EDIT_AF_NAME                         209
#define IDC_STATIC_AF_NAME                       210
#define IDC_DIALOG_AF_VIEW_START                 211
#define IDC_AF_VIEW_AF                           212
#define IDC_AF_VIEW_MD5                          213
#define IDC_AF_VIEW_LINES                        214
#define IDC_AF_VIEW_PHYSICS                      215
#define IDC_CHECK_VIEW_BODIES                    216
#define IDC_CHECK_VIEW_BODYNAMES                 217
#define IDC_CHECK_VIEW_BODYMASS                  218
#define IDC_CHECK_VIEW_TOTALMASS                 219
#define IDC_CHECK_VIEW_INERTIATENSOR             220
#define IDC_CHECK_VIEW_VELOCITY                  221
#define IDC_CHECK_VIEW_CONSTRAINTNAMES           222
#define IDC_CHECK_VIEW_CONSTRAINTS               223
#define IDC_CHECK_VIEW_PRIMARYONLY               224
#define IDC_CHECK_VIEW_LIMITS                    225
#define IDC_CHECK_VIEW_CONSTRAINEDBODIES         226
#define IDC_CHECK_VIEW_TREES                     227
#define IDC_CHECK_MD5_SKELETON                   228
#define IDC_CHECK_MD5_SKELETONONLY               229
#define IDC_CHECK_LINES_DEPTHTEST                230
#define IDC_CHECK_LINES_USEARROWS                231
#define IDC_CHECK_PHYSICS_NOFRICTION             232
#define IDC_CHECK_PHYSICS_NOLIMITS               233
#define IDC_CHECK_PHYSICS_NOGRAVITY              234
#define IDC_CHECK_PHYSICS_NOSELFCOLLISION        235
#define IDC_CHECK_PHYSICS_TIMING                 236
#define IDC_CHECK_PHYSICS_DRAG_ENTITIES          237
#define IDC_CHECK_PHYSICS_SHOW_DRAG_SELECTION    238
#define IDC_DIALOG_AF_PROPERTIES_START           239
#define IDC_STATIC_MD5                           240
#define IDC_STATIC_MODEL                         241
#define IDC_EDIT_MODEL                           242
#define IDC_BUTTON_BROWSE_MODEL                  243
#define IDC_STATIC_SKIN                          244
#define IDC_EDIT_SKIN                            245
#define IDC_BUTTON_BROWSE_SKIN                   246
#define IDC_STATIC_FRICTION                      247
#define IDC_STATIC_LINEARFRICTION                248
#define IDC_EDIT_LINEARFRICTION                  249
#define IDC_SPIN_LINEARFRICTION                  250
#define IDC_STATIC_ANGULARFRICTION               251
#define IDC_EDIT_ANGULARFRICTION                 252
#define IDC_SPIN_ANGULARFRICTION                 253
#define IDC_STATIC_CONTACTFRICTION               254
#define IDC_EDIT_CONTACTFRICTION                 255
#define IDC_SPIN_CONTACTFRICTION                 256
#define IDC_STATIC_CONSTRAINT                    257
#define IDC_EDIT_CONSTRAINTFRICTION              258
#define IDC_SPIN_CONSTRAINTFRICTION              259
#define IDC_STATIC_SUSPENDSPEED                  260
#define IDC_STATIC_COLLISIONDETECTION            261
#define IDC_CHECK_SELFCOLLISION                  262
#define IDC_STATIC_CONTENTS                      263
#define IDC_EDIT_CONTENTS                        264
#define IDC_STATIC_CLIPMASK                      265
#define IDC_EDIT_CLIPMASK                        266
#define IDC_STATIC_LINEARVELOCITY                267
#define IDC_EDIT_LINEARVELOCITY                  268
#define IDC_STATIC_ANGULARVELOCITY               269
#define IDC_EDIT_ANGULARVELOCITY                 270
#define IDC_STATIC_LINEARACCELERATION            271
#define IDC_EDIT_LINEARACCELERATION              272
#define IDC_STATIC_ANGULARACCELERATION           273
#define IDC_EDIT_ANGULARACCELERATION             274
#define IDC_STATIC_MASS                          275
#define IDC_STATIC_TOTALMASS                     276
#define IDC_EDIT_TOTALMASS                       277
#define IDC_SPIN_TOTALMASS                       278
#define IDC_STATIC_SUSPENDMOVEMENT               279
#define IDC_STATIC_LINEAR_TOLERANCE              280
#define IDC_EDIT_LINEAR_TOLERANCE                281
#define IDC_STATIC_ANGULAR_TOLERANCE             282
#define IDC_EDIT_ANGULAR_TOLERANCE               283
#define IDC_STATIC_NO_MOVE_TIME                  284
#define IDC_EDIT_NO_MOVE_TIME                    285
#define IDC_STATIC_MAXIMUM_MOVE_TIME             286
#define IDC_EDIT_MAXIMUM_MOVE_TIME               287
#define IDC_STATIC_MINIMUM_MOVE_TIME             288
#define IDC_EDIT_MINIMUM_MOVE_TIME               289
#define IDC_DIALOG_AF_BODY_START                 290
#define IDC_COMBO_BODIES                         291
#define IDC_BUTTON_NEWBODY                       292
#define IDC_BUTTON_RENAMEBODY                    293
#define IDC_BUTTON_DELETEBODY                    294
#define IDC_BODY_COLLISIONMODEL                  295
#define IDC_STATIC_CM_TYPE                       296
#define IDC_COMBO_CM_TYPE                        297
#define IDC_EDIT_CM_NAME                         298
#define IDC_BUTTON_CM_BROWSE                     299
#define IDC_COMBO_BONE_JOINT1                    300
#define IDC_COMBO_BONE_JOINT2                    301
#define IDC_STATIC_CM_HEIGHT                     302
#define IDC_EDIT_CM_HEIGHT                       303
#define IDC_SPIN_CM_HEIGHT                       304
#define IDC_STATIC_CM_WIDTH                      305
#define IDC_EDIT_CM_WIDTH                        306
#define IDC_SPIN_CM_WIDTH                        307
#define IDC_STATIC_CM_LENGTH                     308
#define IDC_EDIT_CM_LENGTH                       309
#define IDC_SPIN_CM_LENGTH                       310
#define IDC_STATIC_CM_NUMSIDES                   311
#define IDC_EDIT_CM_NUMSIDES                     312
#define IDC_SPIN_CM_NUMSIDES                     313
#define IDC_STATIC_CM_DENSITY                    314
#define IDC_EDIT_CM_DENSITY                      315
#define IDC_SPIN_CM_DENSITY                      316
#define IDC_STATIC_CM_INERTIASCALE               317
#define IDC_EDIT_CM_INERTIASCALE                 318
#define IDC_STATIC_BODY_ORIGIN_AND_ANGLES        319
#define IDC_RADIO_ORIGIN_COORDINATES             320
#define IDC_RADIO_ORIGIN_BONECENTER              321
#define IDC_RADIO_ORIGIN_JOINT                   322
#define IDC_EDIT_AF_VECTOR_X                     323
#define IDC_EDIT_AF_VECTOR_Y                     324
#define IDC_EDIT_AF_VECTOR_Z                     325
#define IDC_SPIN_AF_VECTOR_X                     326
#define IDC_SPIN_AF_VECTOR_Y                     327
#define IDC_SPIN_AF_VECTOR_Z                     328
#define IDC_COMBO_ORIGIN_BONECENTER_JOINT1       329
#define IDC_COMBO_ORIGIN_BONECENTER_JOINT2       330
#define IDC_COMBO_ORIGIN_JOINT                   331
#define IDC_STATIC_CM_ANGLES                     332
#define IDC_STATIC_ANGLES_PITCH                  333
#define IDC_STATIC_ANGLES_YAW                    334
#define IDC_STATIC_ANGLES_ROLL                   335
#define IDC_EDIT_ANGLES_PITCH                    336
#define IDC_SPIN_ANGLES_PITCH                    337
#define IDC_EDIT_ANGLES_YAW                      338
#define IDC_SPIN_ANGLES_YAW                      339
#define IDC_EDIT_ANGLES_ROLL                     340
#define IDC_SPIN_ANGLES_ROLL                     341
#define IDC_STATIC_FRICTIONDIRECTION             342
#define IDC_EDIT_FRICTIONDIRECTION               343
#define IDC_STATIC_CONTACTMOTORDIRECTION         344
#define IDC_EDIT_CONTACTMOTORDIRECTION           345
#define IDC_STATIC_JOINTS                        346
#define IDC_STATIC_MODIFIEDJOINT                 347
#define IDC_COMBO_MODIFIEDJOINT                  348
#define IDC_STATIC_MODIFY                        349
#define IDC_RADIO_MODIFY_ORIENTATION             350
#define IDC_RADIO_MODIFY_POSITION                351
#define IDC_RADIO_MODIFY_BOTH                    352
#define IDC_STATIC_CONTAINEDJOINTS               353
#define IDC_EDIT_CONTAINEDJOINTS                 354
#define IDC_DIALOG_AF_CONSTRAINT_START           355
#define IDC_COMBO_CONSTRAINTS                    356
#define IDC_BUTTON_NEWCONSTRAINT                 357
#define IDC_BUTTON_RENAMECONSTRAINT              358
#define IDC_BUTTON_DELETECONSTRAINT              359
#define IDC_STATIC_CONSTRAINT_GENERAL            360
#define IDC_STATIC_CONSTRAINT_TYPE               361
#define IDC_COMBO_CONSTRAINT_TYPE                362
#define IDC_STATIC_BODY1                         363
#define IDC_STATIC_BODY2                         364
#define IDC_COMBO_CONSTRAINT_BODY1               365
#define IDC_COMBO_CONSTRAINT_BODY2               366
#define IDC_STATIC_CONSTRAINT_FRICTION           367
#define IDC_EDIT_CONSTRAINT_FRICTION             368
#define IDC_SPIN_CONSTRAINT_FRICTION             369
#define IDC_DIALOG_AF_CONSTRAINT_UNIVERSAL_START 370
#define IDC_STATIC_ANCHOR                        371
#define IDC_RADIO_ANCHOR_JOINT                   372
#define IDC_COMBO_ANCHOR_JOINT                   373
#define IDC_RADIO_ANCHOR_COORDINATES             374
#define IDC_EDIT_ANCHOR_X                        375
#define IDC_SPIN_ANCHOR_X                        376
#define IDC_EDIT_ANCHOR_Y                        377
#define IDC_SPIN_ANCHOR_Y                        378
#define IDC_EDIT_ANCHOR_Z                        379
#define IDC_SPIN_ANCHOR_Z                        380
#define IDC_STATIC_UNIVERSAL_SHAFT1              381
#define IDC_RADIO_UNIVERSAL_BONE_SHAFT1          382
#define IDC_RADIO_UNIVERSAL_ANGLES_SHAFT1        383
#define IDC_COMBO_UNIVERSAL_JOINT1_SHAFT1        384
#define IDC_COMBO_UNIVERSAL_JOINT2_SHAFT1        385
#define IDC_EDIT_UNIVERSAL_PITCH_SHAFT1          386
#define IDC_SPIN_UNIVERSAL_PITCH_SHAFT1          387
#define IDC_EDIT_UNIVERSAL_YAW_SHAFT1            388
#define IDC_SPIN_UNIVERSAL_YAW_SHAFT1            389
#define IDC_STATIC_UNIVERSAL_SHAFT2              390
#define IDC_RADIO_UNIVERSAL_BONE_SHAFT2          391
#define IDC_RADIO_UNIVERSAL_ANGLES_SHAFT2        392
#define IDC_COMBO_UNIVERSAL_JOINT1_SHAFT2        393
#define IDC_COMBO_UNIVERSAL_JOINT2_SHAFT2        394
#define IDC_EDIT_UNIVERSAL_PITCH_SHAFT2          395
#define IDC_SPIN_UNIVERSAL_PITCH_SHAFT2          396
#define IDC_EDIT_UNIVERSAL_YAW_SHAFT2            397
#define IDC_SPIN_UNIVERSAL_YAW_SHAFT2            398
#define IDC_STATIC_UNIVERSAL_LIMIT_TYPE          399
#define IDC_RADIO_UNIVERSAL_LIMIT_NONE           400
#define IDC_RADIO_UNIVERSAL_LIMIT_CONE           401
#define IDC_RADIO_UNIVERSAL_LIMIT_PYRAMID        402
#define IDC_STATIC_UNIVERSAL_LIMIT_ORIENTATION   403
#define IDC_RADIO_UNIVERSAL_LIMIT_BONE           404
#define IDC_RADIO_UNIVERSAL_LIMIT_ANGLES         405
#define IDC_COMBO_UNIVERSAL_LIMIT_JOINT1         406
#define IDC_COMBO_UNIVERSAL_LIMIT_JOINT2         407
#define IDC_EDIT_UNIVERSAL_LIMIT_YAW             408
#define IDC_SPIN_UNIVERSAL_LIMIT_YAW             409
#define IDC_EDIT_UNIVERSAL_LIMIT_PITCH           410
#define IDC_SPIN_UNIVERSAL_LIMIT_PITCH           411
#define IDC_EDIT_UNIVERSAL_LIMIT_ROLL            412
#define IDC_SPIN_UNIVERSAL_LIMIT_ROLL            413
#define IDC_EDIT_UNIVERSAL_LIMIT_CONE_ANGLE      414
#define IDC_SPIN_UNIVERSAL_LIMIT_CONE_ANGLE      415
#define IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE1  416
#define IDC_SPIN_UNIVERSAL_LIMIT_PYRAMID_ANGLE1  417
#define IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE2  418
#define IDC_SPIN_UNIVERSAL_LIMIT_PYRAMID_ANGLE2  419
#define IDC_DIALOG_AF_CONSTRAINT_HINGE_START     420
#define IDC_STATIC_HINGE_AXIS                    421
#define IDC_RADIO_HINGE_AXIS_BONE                422
#define IDC_RADIO_HINGE_AXIS_ANGLES              423
#define IDC_COMBO_HINGE_AXIS_JOINT1              424
#define IDC_COMBO_HINGE_AXIS_JOINT2              425
#define IDC_EDIT_HINGE_AXIS_PITCH                426
#define IDC_SPIN_HINGE_AXIS_PITCH                427
#define IDC_EDIT_HINGE_AXIS_YAW                  428
#define IDC_SPIN_HINGE_AXIS_YAW                  429
#define IDC_STATIC_HINGE_LIMIT                   430
#define IDC_RADIO_HINGE_LIMIT_NONE               431
#define IDC_STATIC_HINGE_LIMIT2                  432
#define IDC_RADIO_HINGE_LIMIT_ANGLES             433
#define IDC_SPIN_HINGE_LIMIT_ANGLE1              434
#define IDC_EDIT_HINGE_LIMIT_ANGLE1              435
#define IDC_SPIN_HINGE_LIMIT_ANGLE2              436
#define IDC_EDIT_HINGE_LIMIT_ANGLE2              437
#define IDC_EDIT_HINGE_LIMIT_ANGLE3              438
#define IDC_SPIN_HINGE_LIMIT_ANGLE3              439
#define IDC_DIALOG_AF_CONSTRAINT_BAS_START       440
#define IDC_STATIC_BAS_LIMIT_TYPE                441
#define IDC_RADIO_BAS_LIMIT_NONE                 442
#define IDC_RADIO_BAS_LIMIT_CONE                 443
#define IDC_RADIO_BAS_LIMIT_PYRAMID              444
#define IDC_EDIT_BAS_LIMIT_CONE_ANGLE            445
#define IDC_SPIN_BAS_LIMIT_CONE_ANGLE            446
#define IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE1        447
#define IDC_SPIN_BAS_LIMIT_PYRAMID_ANGLE1        448
#define IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE2        449
#define IDC_SPIN_BAS_LIMIT_PYRAMID_ANGLE2        450
#define IDC_EDIT_BAS_LIMIT_ROLL                  451
#define IDC_SPIN_BAS_LIMIT_ROLL                  452
#define IDC_STATIC_BAS_LIMIT_ORIENTATION         453
#define IDC_RADIO_BAS_LIMIT_BONE                 454
#define IDC_RADIO_BAS_LIMIT_ANGLES               455
#define IDC_COMBO_BAS_LIMIT_JOINT1               456
#define IDC_COMBO_BAS_LIMIT_JOINT2               457
#define IDC_EDIT_BAS_LIMIT_PITCH                 458
#define IDC_SPIN_BAS_LIMIT_PITCH                 459
#define IDC_EDIT_BAS_LIMIT_YAW                   460
#define IDC_SPIN_BAS_LIMIT_YAW                   461
#define IDC_STATIC_BAS_LIMIT_AXIS                462
#define IDC_RADIO_BAS_LIMIT_AXIS_BONE            463
#define IDC_COMBO_BAS_LIMIT_AXIS_JOINT1          464
#define IDC_COMBO_BAS_LIMIT_AXIS_JOINT2          465
#define IDC_RADIO_BAS_LIMIT_AXIS_ANGLES          466
#define IDC_EDIT_BAS_LIMIT_AXIS_PITCH            467
#define IDC_SPIN_BAS_LIMIT_AXIS_PITCH            468
#define IDC_EDIT_BAS_LIMIT_AXIS_YAW              469
#define IDC_SPIN_BAS_LIMIT_AXIS_YAW              470
#define IDC_DIALOG_AF_CONSTRAINT_SLIDER_START    471
#define IDC_RADIO_SLIDER_AXIS_BONE               472
#define IDC_COMBO_SLIDER_AXIS_JOINT1             473
#define IDC_COMBO_SLIDER_AXIS_JOINT2             474
#define IDC_RADIO_SLIDER_AXIS_ANGLES             475
#define IDC_EDIT_SLIDER_AXIS_PITCH               476
#define IDC_SPIN_SLIDER_AXIS_PITCH               477
#define IDC_EDIT_SLIDER_AXIS_YAW                 478
#define IDC_SPIN_SLIDER_AXIS_YAW                 479
#define IDC_STATIC_SLIDER_AXIS                   480
#define IDC_DIALOG_AF_CONSTRAINT_SPRING_START    481
#define IDC_STATIC_ANCHOR2                       482
#define IDC_RADIO_ANCHOR2_JOINT                  483
#define IDC_COMBO_ANCHOR2_JOINT                  484
#define IDC_RADIO_ANCHOR2_COORDINATES            485
#define IDC_EDIT_ANCHOR2_X                       486
#define IDC_SPIN_ANCHOR2_X                       487
#define IDC_EDIT_ANCHOR2_Y                       488
#define IDC_SPIN_ANCHOR2_Y                       489
#define IDC_EDIT_ANCHOR2_Z                       490
#define IDC_SPIN_ANCHOR2_Z                       491
#define IDC_STATIC_SPRING_SETTINGS               492
#define IDC_STATIC_SPRING_STRETCH                493
#define IDC_STATIC_SPRING_COMPRESS               494
#define IDC_STATIC_SPRING_DAMPING                495
#define IDC_STATIC_SPRING_REST_LENGTH            496
#define IDC_EDIT_SPRING_STRETCH                  497
#define IDC_SPIN_SPRING_STRETCH                  498
#define IDC_EDIT_SPRING_COMPRESS                 499
#define IDC_SPIN_SPRING_COMPRESS                 500
#define IDC_EDIT_SPRING_DAMPING                  501
#define IDC_SPIN_SPRING_DAMPING                  502
#define IDC_EDIT_SPRING_REST_LENGTH              503
#define IDC_SPIN_SPRING_REST_LENGTH              504
#define IDC_STATIC_SPRING_LIMIT                  505
#define IDC_RADIO_SPRING_NO_MIN_LENGTH           506
#define IDC_RADIO_SPRING_MIN_LENGTH              507
#define IDC_EDIT_SPRING_MIN_LENGTH               508
#define IDC_SPIN_SPRING_MIN_LENGTH               509
#define IDC_RADIO_SPRING_NO_MAX_LENGTH           510
#define IDC_RADIO_SPRING_MAX_LENGTH              511
#define IDC_EDIT_SPRING_MAX_LENGTH               512
#define IDC_SPIN_SPRING_MAX_LENGTH               513


// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_3D_CONTROLS                1
#define _APS_NEXT_RESOURCE_VALUE        113
#define _APS_NEXT_COMMAND_VALUE         20000
#define _APS_NEXT_CONTROL_VALUE         514
#define _APS_NEXT_SYMED_VALUE           113
#endif
#endif

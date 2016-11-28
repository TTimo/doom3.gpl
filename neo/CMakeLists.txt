if( NOT SDL2_DIR )
	Message( "SDL2_DIR not set, making platform specific build" )
endif()

MESSAGE( STATUS ${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} )
project(Doom3)
cmake_minimum_required(VERSION 2.8)
set( APPLICATION_NAME "Doom 3" )

set( BUILD_SHARED_LIBS OFF )
set( CMAKE_INSTALL_PREFIX $ENV{HOME}/Desktop )
set( DATA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Data )
set( DATA_INSTALL_ROOT ${CMAKE_INSTALL_PREFIX}/Contents/Resources )
set( BINARY_INSTALL_ROOT ${CMAKE_INSTALL_PREFIX}/Contents/MacOS )
set( CMAKE_OSX_ARCHITECTURES "i386" )
set( CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin )
set( CMAKE_OSX_DEPLOYMENT_TARGET 10.11 )

if( SDL2_DIR )
	set( SDL2_INCLUDE_DIR ${SDL2_DIR}/include/ )
	set( SDL2_LIBRARIES   ${SDL2_DIR}/lib/libSDL2.a )
	include_directories( ${SDL2_INCLUDE_DIR} )
endif()

# Apple projects are unique RUNTIME_OUTPUT_DIRECTORY is not actually where the executable goes, also the xcode generator creates
# release and Debug directories not present in the makefile project
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
	if(APPLE)
		set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/${APPLICATION_NAME}.app/Contents/MacOS" )
	endif(APPLE)
elseif(CMAKE_GENERATOR STREQUAL "Xcode")
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../bin/Debug/${APPLICATION_NAME}.app/Contents/MacOS )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release/${APPLICATION_NAME}.app/Contents/MacOS )
endif()

IF( APPLE )
	ADD_DEFINITIONS( -DMACOS_X -DDEBUG )
	ADD_DEFINITIONS( -g )
	SET( CMAKE_CXX_FLAGS "-Wno-deprecated-writable-strings -Wno-parentheses-equality -Wno-format -Wno-switch -Wno-null-arithmetic -Wno-bitwise-op-parentheses -Wno-unused-value -Wno-sizeof-pointer-memaccess -Wno-enum-compare  -Wno-delete-non-virtual-dtor -Wno-unused-variable -fvisibility=hidden" )
ELSE()
	set (CMAKE_MFC_FLAG 2)
	ADD_DEFINITIONS( -D_WIN32 -DDEBUG )
	SET( CPP_WARNING_OPTIONS -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DNO_WARN_MBCS_MFC_DEPRECATION )
ENDIF()

find_package(OpenGL)

add_subdirectory(idlib)
add_subdirectory(game)
add_subdirectory(d3xp)
add_subdirectory(sound)
IF(WIN32)
add_subdirectory(curl)
ENDIF()

SET( RENDERER_SRC
	./renderer/Cinematic.cpp
	./renderer/GuiModel.cpp
	./renderer/Image_files.cpp
	./renderer/Image_init.cpp
	./renderer/Image_load.cpp
	./renderer/Image_process.cpp
	./renderer/Image_program.cpp
	./renderer/Interaction.cpp
	./renderer/Material.cpp
	./renderer/MegaTexture.cpp
	./renderer/Model.cpp
	./renderer/ModelDecal.cpp
	./renderer/ModelManager.cpp
	./renderer/ModelOverlay.cpp
	./renderer/Model_ase.cpp
	./renderer/Model_beam.cpp
	./renderer/Model_liquid.cpp
	./renderer/Model_lwo.cpp
	./renderer/Model_ma.cpp
	./renderer/Model_md3.cpp
	./renderer/Model_md5.cpp
	./renderer/Model_prt.cpp
	./renderer/Model_sprite.cpp
	./renderer/RenderEntity.cpp
	./renderer/RenderSystem.cpp
	./renderer/RenderSystem_init.cpp
	./renderer/RenderWorld.cpp
	./renderer/RenderWorld_demo.cpp
	./renderer/RenderWorld_load.cpp
	./renderer/RenderWorld_portals.cpp
	./renderer/VertexCache.cpp
	./renderer/cg_explicit.cpp
	./renderer/draw_arb.cpp
	./renderer/draw_arb2.cpp
	./renderer/draw_common.cpp
	./renderer/draw_exp_stub.cpp
	./renderer/draw_nv10.cpp
	./renderer/draw_nv20.cpp
	./renderer/draw_r200.cpp
	./renderer/tr_backend.cpp
	./renderer/tr_deform.cpp
	./renderer/tr_font.cpp
	./renderer/tr_guisurf.cpp
	./renderer/tr_light.cpp
	./renderer/tr_lightrun.cpp
	./renderer/tr_main.cpp
	./renderer/tr_orderIndexes.cpp
	./renderer/tr_polytope.cpp
	./renderer/tr_render.cpp
	./renderer/tr_rendertools.cpp
	./renderer/tr_shadowbounds.cpp
	./renderer/tr_stencilshadow.cpp
	./renderer/tr_subview.cpp
	./renderer/tr_trace.cpp
	./renderer/tr_trisurf.cpp
	./renderer/tr_turboshadow.cpp
    ./renderer/jpeg-6/jcapimin.c
    ./renderer/jpeg-6/jcapistd.c
    ./renderer/jpeg-6/jccoefct.c
    ./renderer/jpeg-6/jccolor.c
    ./renderer/jpeg-6/jcdctmgr.c
	./renderer/jpeg-6/jchuff.c
	./renderer/jpeg-6/jcinit.c
	./renderer/jpeg-6/jcmainct.c
	./renderer/jpeg-6/jcmarker.c
	./renderer/jpeg-6/jcmaster.c
	./renderer/jpeg-6/jcomapi.c
	./renderer/jpeg-6/jcparam.c
	./renderer/jpeg-6/jcphuff.c
	./renderer/jpeg-6/jcprepct.c
	./renderer/jpeg-6/jcsample.c
	./renderer/jpeg-6/jctrans.c
	./renderer/jpeg-6/jdapimin.c
	./renderer/jpeg-6/jdapistd.c
	./renderer/jpeg-6/jdatadst.c
	./renderer/jpeg-6/jdatasrc.c
	./renderer/jpeg-6/jdcoefct.c
	./renderer/jpeg-6/jdcolor.c
	./renderer/jpeg-6/jddctmgr.c
	./renderer/jpeg-6/jdhuff.c
	./renderer/jpeg-6/jdinput.c
	./renderer/jpeg-6/jdmainct.c
	./renderer/jpeg-6/jdmarker.c
	./renderer/jpeg-6/jdmaster.c
	./renderer/jpeg-6/jdmerge.c
	./renderer/jpeg-6/jdphuff.c
	./renderer/jpeg-6/jdpostct.c
	./renderer/jpeg-6/jdsample.c
	./renderer/jpeg-6/jdtrans.c
	./renderer/jpeg-6/jerror.c
	./renderer/jpeg-6/jfdctflt.c
	./renderer/jpeg-6/jfdctfst.c
	./renderer/jpeg-6/jfdctint.c
	./renderer/jpeg-6/jidctflt.c
	./renderer/jpeg-6/jidctfst.c
	./renderer/jpeg-6/jidctint.c
	./renderer/jpeg-6/jidctred.c
	./renderer/jpeg-6/jmemmgr.c
	./renderer/jpeg-6/jmemnobs.c
	./renderer/jpeg-6/jquant1.c
	./renderer/jpeg-6/jquant2.c
	./renderer/jpeg-6/jutils.c
)

SET( TOOLS_SRC 
	./tools/af/DialogAF.cpp
	./tools/af/DialogAF.h
	./tools/af/DialogAFBody.cpp
	./tools/af/DialogAFBody.h
	./tools/af/DialogAFConstraint.cpp
	./tools/af/DialogAFConstraint.h
	./tools/af/DialogAFConstraintBallAndSocket.cpp
	./tools/af/DialogAFConstraintBallAndSocket.h
	./tools/af/DialogAFConstraintFixed.cpp
	./tools/af/DialogAFConstraintFixed.h
	./tools/af/DialogAFConstraintHinge.cpp
	./tools/af/DialogAFConstraintHinge.h
	./tools/af/DialogAFConstraintSlider.cpp
	./tools/af/DialogAFConstraintSlider.h
	./tools/af/DialogAFConstraintSpring.cpp
	./tools/af/DialogAFConstraintSpring.h
	./tools/af/DialogAFConstraintUniversal.cpp
	./tools/af/DialogAFConstraintUniversal.h
	./tools/af/DialogAFName.cpp
	./tools/af/DialogAFName.h
	./tools/af/DialogAFProperties.cpp
	./tools/af/DialogAFProperties.h
	./tools/af/DialogAFView.cpp
	./tools/af/DialogAFView.h
	./tools/comafx/CDIB.cpp
	./tools/comafx/CDIB.h
	./tools/comafx/CPathTreeCtrl.cpp
	./tools/comafx/CPathTreeCtrl.h
	./tools/comafx/CSyntaxRichEditCtrl.cpp
	./tools/comafx/CSyntaxRichEditCtrl.h
	./tools/comafx/DialogColorPicker.cpp
	./tools/comafx/DialogColorPicker.h
	./tools/comafx/DialogGoToLine.cpp
	./tools/comafx/DialogGoToLine.h
	./tools/comafx/DialogName.cpp
	./tools/comafx/DialogName.h
	./tools/comafx/StdAfx.cpp
	./tools/comafx/StdAfx.h
	./tools/comafx/VectorCtl.cpp
	./tools/comafx/VectorCtl.h
	./tools/common/AlphaPopup.cpp
	./tools/common/ColorButton.cpp
	./tools/common/ColorButton.h
	./tools/common/DialogHelpers.h
	./tools/common/MaskEdit.cpp
	./tools/common/MaskEdit.h
	./tools/common/OpenFileDialog.cpp
	./tools/common/OpenFileDialog.h
	./tools/common/PropertyGrid.cpp
	./tools/common/PropertyGrid.h
	./tools/common/PropTree/PropTree.cpp
	./tools/common/PropTree/PropTree.h
	./tools/common/PropTree/PropTreeInfo.cpp
	./tools/common/PropTree/PropTreeInfo.h
	./tools/common/PropTree/PropTreeItem.cpp
	./tools/common/PropTree/PropTreeItem.h
	./tools/common/PropTree/PropTreeItemButton.cpp
	./tools/common/PropTree/PropTreeItemButton.h
	./tools/common/PropTree/PropTreeItemCheck.cpp
	./tools/common/PropTree/PropTreeItemCheck.h
	./tools/common/PropTree/PropTreeItemColor.cpp
	./tools/common/PropTree/PropTreeItemColor.h
	./tools/common/PropTree/PropTreeItemCombo.cpp
	./tools/common/PropTree/PropTreeItemCombo.h
	./tools/common/PropTree/PropTreeItemEdit.cpp
	./tools/common/PropTree/PropTreeItemEdit.h
	./tools/common/PropTree/PropTreeItemEditButton.cpp
	./tools/common/PropTree/PropTreeItemEditButton.h
	./tools/common/PropTree/PropTreeItemFileEdit.cpp
	./tools/common/PropTree/PropTreeItemFileEdit.h
	./tools/common/PropTree/PropTreeItemStatic.cpp
	./tools/common/PropTree/PropTreeItemStatic.h
	./tools/common/PropTree/PropTreeList.cpp
	./tools/common/PropTree/PropTreeList.h
	./tools/common/PropTree/PropTreeView.cpp
	./tools/common/PropTree/PropTreeView.h
	./tools/common/RegistryOptions.cpp
	./tools/common/RegistryOptions.h
	./tools/common/RollupPanel.cpp
	./tools/common/RollupPanel.h
	./tools/common/SpinButton.cpp
	./tools/common/SpinButton.h
	./tools/compilers/renderbump/renderbump.cpp
	./tools/decl/DialogDeclBrowser.cpp
	./tools/decl/DialogDeclBrowser.h
	./tools/decl/DialogDeclEditor.cpp
	./tools/decl/DialogDeclEditor.h
	./tools/decl/DialogDeclNew.cpp
	./tools/decl/DialogDeclNew.h
	./tools/guied/GEApp.cpp
	./tools/guied/GEApp.h
	./tools/guied/GECheckInDlg.cpp
	./tools/guied/GEDeleteModifier.cpp
	./tools/guied/GEDeleteModifier.h
	./tools/guied/GEHideModifier.cpp
	./tools/guied/GEHideModifier.h
	./tools/guied/GEInsertModifier.cpp
	./tools/guied/GEInsertModifier.h
	./tools/guied/GEItemPropsDlg.cpp
	./tools/guied/GEItemPropsDlg.h
	./tools/guied/GEItemScriptsDlg.cpp
	./tools/guied/GEItemScriptsDlg.h
	./tools/guied/GEKeyValueModifier.cpp
	./tools/guied/GEKeyValueModifier.h
	./tools/guied/GEModifier.cpp
	./tools/guied/GEModifier.h
	./tools/guied/GEModifierGroup.cpp
	./tools/guied/GEModifierGroup.h
	./tools/guied/GEModifierStack.cpp
	./tools/guied/GEModifierStack.h
	./tools/guied/GEMoveModifier.cpp
	./tools/guied/GEMoveModifier.h
	./tools/guied/GENavigator.cpp
	./tools/guied/GENavigator.h
	./tools/guied/GEOptions.cpp
	./tools/guied/GEOptions.h
	./tools/guied/GEOptionsDlg.cpp
	./tools/guied/GEOptionsDlg.h
	./tools/guied/GEProperties.cpp
	./tools/guied/GEProperties.h
	./tools/guied/GEPropertyPage.cpp
	./tools/guied/GEPropertyPage.h
	./tools/guied/GESelectionMgr.cpp
	./tools/guied/GESelectionMgr.h
	./tools/guied/GESizeModifier.cpp
	./tools/guied/GESizeModifier.h
	./tools/guied/GEStateModifier.cpp
	./tools/guied/GEStateModifier.h
	./tools/guied/GEStatusBar.cpp
	./tools/guied/GEStatusBar.h
	./tools/guied/GETransformer.cpp
	./tools/guied/GETransformer.h
	./tools/guied/GEViewer.cpp
	./tools/guied/GEViewer.h
	./tools/guied/GEWindowWrapper.cpp
	./tools/guied/GEWindowWrapper.h
	./tools/guied/GEWorkspace.cpp
	./tools/guied/GEWorkspace.h
	./tools/guied/GEWorkspaceFile.cpp
	./tools/guied/GEZOrderModifier.cpp
	./tools/guied/GEZOrderModifier.h
	./tools/guied/guied.cpp
	./tools/materialeditor/ConsoleView.cpp
	./tools/materialeditor/ConsoleView.h
	./tools/materialeditor/FindDialog.cpp
	./tools/materialeditor/FindDialog.h
	./tools/materialeditor/MaterialDef.cpp
	./tools/materialeditor/MaterialDef.h
	./tools/materialeditor/MaterialDoc.cpp
	./tools/materialeditor/MaterialDoc.h
	./tools/materialeditor/MaterialDocManager.cpp
	./tools/materialeditor/MaterialDocManager.h
	./tools/materialeditor/MaterialEditor.cpp
	./tools/materialeditor/MaterialEditor.h
	./tools/materialeditor/MaterialEditView.cpp
	./tools/materialeditor/MaterialEditView.h
	./tools/materialeditor/MaterialModifier.cpp
	./tools/materialeditor/MaterialModifier.h
	./tools/materialeditor/MaterialPreviewPropView.cpp
	./tools/materialeditor/MaterialPreviewPropView.h
	./tools/materialeditor/MaterialPreviewView.cpp
	./tools/materialeditor/MaterialPreviewView.h
	./tools/materialeditor/MaterialPropTreeView.cpp
	./tools/materialeditor/MaterialPropTreeView.h
	./tools/materialeditor/MaterialTreeView.cpp
	./tools/materialeditor/MaterialTreeView.h
	./tools/materialeditor/MaterialView.cpp
	./tools/materialeditor/MaterialView.h
	./tools/materialeditor/MEMainFrame.cpp
	./tools/materialeditor/MEMainFrame.h
	./tools/materialeditor/MEOptions.cpp
	./tools/materialeditor/MEOptions.h
	./tools/materialeditor/StageView.cpp
	./tools/materialeditor/StageView.h
	./tools/materialeditor/ToggleListView.cpp
	./tools/materialeditor/ToggleListView.h
	./tools/particle/DialogParticleEditor.cpp
	./tools/particle/DialogParticleEditor.h
	./tools/pda/DialogPDAEditor.cpp
	./tools/pda/DialogPDAEditor.h
	./tools/radiant/autocaulk.cpp
	./tools/radiant/autocaulk.h
	./tools/radiant/CameraTargetDlg.cpp
	./tools/radiant/CameraTargetDlg.h
	./tools/radiant/CamWnd.cpp
	./tools/radiant/CamWnd.h
	./tools/radiant/CapDialog.cpp
	./tools/radiant/CapDialog.h
	./tools/radiant/cmdlib.cpp
	./tools/radiant/cmdlib.h
	./tools/radiant/CommandsDlg.cpp
	./tools/radiant/CommandsDlg.h
	./tools/radiant/CommentsDlg.cpp
	./tools/radiant/CommentsDlg.h
	./tools/radiant/ConsoleDlg.cpp
	./tools/radiant/ConsoleDlg.h
	./tools/radiant/CSG.CPP
	./tools/radiant/CurveDlg.cpp
	./tools/radiant/CurveDlg.h
	./tools/radiant/DialogInfo.cpp
	./tools/radiant/DialogInfo.h
	./tools/radiant/DialogTextures.cpp
	./tools/radiant/DialogTextures.h
	./tools/radiant/DialogThick.cpp
	./tools/radiant/DialogThick.h
	./tools/radiant/DlgCamera.cpp
	./tools/radiant/DlgCamera.h
	./tools/radiant/DlgEvent.cpp
	./tools/radiant/DlgEvent.h
	./tools/radiant/DRAG.CPP
	./tools/radiant/ECLASS.CPP
	./tools/radiant/EditorBrush.cpp
	./tools/radiant/EditorBrush.h
	./tools/radiant/EditorBrushPrimit.cpp
	./tools/radiant/EditorEntity.cpp
	./tools/radiant/EditorEntity.h
	./tools/radiant/EditorMap.cpp
	./tools/radiant/EditorMap.h
	./tools/radiant/EditViewDlg.cpp
	./tools/radiant/EditViewDlg.h
	./tools/radiant/EntityDlg.cpp
	./tools/radiant/EntityDlg.h
	./tools/radiant/EntityListDlg.cpp
	./tools/radiant/EntityListDlg.h
	./tools/radiant/EntKeyFindReplace.cpp
	./tools/radiant/EntKeyFindReplace.h
	./tools/radiant/FindTextureDlg.cpp
	./tools/radiant/FindTextureDlg.h
	./tools/radiant/GetString.cpp
	./tools/radiant/GetString.h
	./tools/radiant/GLWidget.cpp
	./tools/radiant/GLWidget.h
	./tools/radiant/InspectorDialog.cpp
	./tools/radiant/InspectorDialog.h
	./tools/radiant/LightDlg.cpp
	./tools/radiant/LightDlg.h
	./tools/radiant/MainFrm.cpp
	./tools/radiant/MainFrm.h
	./tools/radiant/MapInfo.cpp
	./tools/radiant/MapInfo.h
	./tools/radiant/MediaPreviewDlg.cpp
	./tools/radiant/MediaPreviewDlg.h
	./tools/radiant/MRU.CPP
	./tools/radiant/MRU.H
	./tools/radiant/NewProjDlg.cpp
	./tools/radiant/NewProjDlg.h
	./tools/radiant/NewTexWnd.cpp
	./tools/radiant/NewTexWnd.h
	./tools/radiant/PARSE.CPP
	./tools/radiant/PARSE.H
	./tools/radiant/PatchDensityDlg.cpp
	./tools/radiant/PatchDensityDlg.h
	./tools/radiant/PatchDialog.cpp
	./tools/radiant/PatchDialog.h
	./tools/radiant/PMESH.CPP
	./tools/radiant/PMESH.H
	./tools/radiant/PointFile.cpp
	./tools/radiant/PrefsDlg.cpp
	./tools/radiant/PrefsDlg.h
	./tools/radiant/PreviewDlg.cpp
	./tools/radiant/PreviewDlg.h
	./tools/radiant/PropertyList.cpp
	./tools/radiant/PropertyList.h
	./tools/radiant/QE3.CPP
	./tools/radiant/QE3.H
	./tools/radiant/QEDEFS.H
	./tools/radiant/QERTYPES.H
	./tools/radiant/Radiant.cpp
	./tools/radiant/Radiant.h
	./tools/radiant/RotateDlg.cpp
	./tools/radiant/RotateDlg.h
	./tools/radiant/ScaleDialog.cpp
	./tools/radiant/ScaleDialog.h
	./tools/radiant/SELECT.CPP
	./tools/radiant/SELECT.H
	./tools/radiant/splines.cpp
	./tools/radiant/splines.h
	./tools/radiant/SurfaceDlg.cpp
	./tools/radiant/SurfaceDlg.h
	./tools/radiant/TabsDlg.cpp
	./tools/radiant/TabsDlg.h
	./tools/radiant/TearoffContainerWindow.cpp
	./tools/radiant/TearoffContainerWindow.h
	./tools/radiant/TextureBar.cpp
	./tools/radiant/TextureBar.h
	./tools/radiant/Textures.h
	./tools/radiant/Undo.cpp
	./tools/radiant/Undo.h
	./tools/radiant/VERTSEL.CPP
	./tools/radiant/WaitDlg.cpp
	./tools/radiant/WaitDlg.h
	./tools/radiant/WaveOpen.cpp
	./tools/radiant/WaveOpen.h
	./tools/radiant/WIN_DLG.CPP
	./tools/radiant/WIN_QE3.CPP
	./tools/radiant/XYWnd.cpp
	./tools/radiant/XYWnd.h
	./tools/radiant/Z.CPP
	./tools/radiant/Z.H
	./tools/radiant/ZClip.cpp
	./tools/radiant/ZClip.h
	./tools/radiant/ZWnd.cpp
	./tools/radiant/ZWnd.h
	./tools/script/DialogScriptEditor.cpp
	./tools/script/DialogScriptEditor.h
	./tools/sound/DialogSound.cpp
	./tools/sound/DialogSound.h
	./tools/sound/DialogSoundGroup.cpp
	./tools/sound/DialogSoundGroup.h
)

SET( MACOSX_SRC
	./sys/osx/DOOMController.mm
	./sys/osx/macosx_event.mm
	./sys/osx/macosx_glimp.mm
	./sys/osx/macosx_misc.mm
	./sys/posix/posix_input.cpp
	./sys/posix/posix_main.cpp
	./sys/posix/posix_net.cpp
	./sys/posix/posix_signal.cpp
	./sys/posix/posix_threads.cpp
	./sys/linux/stack.cpp
	./sys/osx/PreferencesDialog.cpp
	./sys/osx/macosx_guids.cpp
	./sys/osx/macosx_sound.cpp
)

SET( WIN32_SRC
	./sys/win32/rc/CreateResourceIDs.cpp
	./sys/win32/win_cpu.cpp
	./sys/win32/win_glimp.cpp
	./sys/win32/win_input.cpp
	./sys/win32/win_main.cpp
	./sys/win32/win_net.cpp
	./sys/win32/win_qgl.cpp
	./sys/win32/win_shared.cpp
	./sys/win32/win_snd.cpp
	./sys/win32/win_syscon.cpp
	./sys/win32/win_taskkeyhook.cpp
	./sys/win32/win_wndproc.cpp
)

SET( LINUX_SRC 
	./sys/posix/posix_input.cpp
	./sys/posix/posix_main.cpp
	./sys/posix/posix_net.cpp
	./sys/posix/posix_signal.cpp
	./sys/posix/posix_threads.cpp
	./sys/linux/dedicated.cpp
	./sys/linux/glimp.cpp
	./sys/linux/input.cpp
	./sys/linux/main.cpp
	./sys/linux/sound.cpp
	./sys/linux/sound_alsa.cpp
	./sys/linux/stack.cpp
)

SET( SDL2_SRC
	./sys/sdl2/sdl2_glimp.cpp
	./sys/sdl2/sdl2_main.cpp
	./sys/sdl2/sdl2_input.cpp
	./sys/sdl2/sdl2_snd.cpp
	./sys/sdl2/sdl2_shared.cpp
	./sys/sdl2/sdl2_cpu.cpp
	./sys/sdl2/sdl2_net.cpp
	./sys/sdl2/sdl2_threads.cpp
)

SET( SOUND_SRC
	./sound/snd_cache.cpp
	./sound/snd_decoder.cpp
	./sound/snd_efxfile.cpp
	./sound/snd_emitter.cpp
	./sound/snd_shader.cpp
	./sound/snd_system.cpp
	./sound/snd_wavefile.cpp
	./sound/snd_world.cpp
)

SET( FRAMEWORK_SRC 
	./framework/CVarSystem.cpp
	./framework/CmdSystem.cpp
	./framework/Common.cpp
	./framework/Compressor.cpp
	./framework/Console.cpp
	./framework/DeclAF.cpp
	./framework/DeclEntityDef.cpp
	./framework/DeclFX.cpp
	./framework/DeclManager.cpp
	./framework/DeclPDA.cpp
	./framework/DeclParticle.cpp
	./framework/DeclSkin.cpp
	./framework/DeclTable.cpp
	./framework/DemoFile.cpp
	./framework/EditField.cpp
	./framework/EventLoop.cpp
	./framework/File.cpp
	./framework/FileSystem.cpp
	./framework/KeyInput.cpp
	./framework/Session.cpp
	./framework/Session_menu.cpp
	./framework/Unzip.cpp
	./framework/UsercmdGen.cpp
	./framework/async/AsyncClient.cpp
	./framework/async/AsyncNetwork.cpp
	./framework/async/AsyncServer.cpp
	./framework/async/MsgChannel.cpp
	./framework/async/NetworkSystem.cpp
	./framework/async/ServerScan.cpp
)

SET( UI_SRC
	./ui/BindWindow.cpp
	./ui/ChoiceWindow.cpp
	./ui/DeviceContext.cpp
	./ui/EditWindow.cpp
	./ui/FieldWindow.cpp
	./ui/GameBearShootWindow.cpp
	./ui/GameBustOutWindow.cpp
	./ui/GameSSDWindow.cpp
	./ui/GameWindow.cpp
	./ui/GuiScript.cpp
	./ui/ListGUI.cpp
	./ui/ListWindow.cpp
	./ui/MarkerWindow.cpp
	./ui/RegExp.cpp
	./ui/RenderWindow.cpp
	./ui/SimpleWindow.cpp
	./ui/SliderWindow.cpp
	./ui/UserInterface.cpp
	./ui/Window.cpp
	./ui/Winvar.cpp
)

SET( CM_SRC
	./cm/CollisionModel_contacts.cpp
	./cm/CollisionModel_contents.cpp
	./cm/CollisionModel_debug.cpp
	./cm/CollisionModel_files.cpp
	./cm/CollisionModel_load.cpp
	./cm/CollisionModel_rotate.cpp
	./cm/CollisionModel_trace.cpp
	./cm/CollisionModel_translate.cpp
)

SET( GAMETOOLS_SRC
	./tools/compilers/aas/AASBuild.cpp
	./tools/compilers/aas/AASBuild_file.cpp
	./tools/compilers/aas/AASBuild_gravity.cpp
	./tools/compilers/aas/AASBuild_ledge.cpp
	./tools/compilers/aas/AASBuild_merge.cpp
	./tools/compilers/aas/AASCluster.cpp
	./tools/compilers/aas/AASFile.cpp
	./tools/compilers/aas/AASFileManager.cpp
	./tools/compilers/aas/AASFile_optimize.cpp
	./tools/compilers/aas/AASFile_sample.cpp
	./tools/compilers/aas/AASReach.cpp
	./tools/compilers/aas/Brush.cpp
	./tools/compilers/aas/BrushBSP.cpp
	./tools/compilers/dmap/dmap.cpp
	./tools/compilers/dmap/facebsp.cpp
	./tools/compilers/dmap/gldraw.cpp
	./tools/compilers/dmap/glfile.cpp
	./tools/compilers/dmap/leakfile.cpp
	./tools/compilers/dmap/map.cpp
	./tools/compilers/dmap/optimize.cpp
	./tools/compilers/dmap/optimize_gcc.cpp
	./tools/compilers/dmap/output.cpp
	./tools/compilers/dmap/portals.cpp
	./tools/compilers/dmap/shadowopt3.cpp
	./tools/compilers/dmap/tritjunction.cpp
	./tools/compilers/dmap/tritools.cpp
	./tools/compilers/dmap/ubrush.cpp
	./tools/compilers/dmap/usurface.cpp
	./tools/compilers/renderbump/renderbump.cpp
	./tools/compilers/roqvq/NSBitmapImageRep.cpp
	./tools/compilers/roqvq/codec.cpp
	./tools/compilers/roqvq/roq.cpp
	./tools/compilers/roqvq/roqParam.cpp
	./tools/guied/GEWindowWrapper_stub.cpp
)

SET( COMMON_SRC 
	./sys/sys_local.cpp
	${SOUND_SRC}
	${FRAMEWORK_SRC}
	${UI_SRC}
	${CM_SRC}
	${GAMETOOLS_SRC}
	${RENDERER_SRC}
)

ADD_DEFINITIONS( -DID_ENFORCE_KEY=0 -D__DOOM_DLL__ -DID_ENABLE_CURL=1 )

IF( APPLE )
	SET( FRAMEWORKS "-framework Carbon -framework Cocoa -framework OpenGL -framework OpenAL -framework IOKit -framework CoreAudio -framework CoreGraphics -framework Foundation -framework AudioUnit -framework ForceFeedback" )
	IF( SDL2_DIR )
		SET( SOURCES ${COMMON_SRC} ${SDL2_SRC} 	./sys/sdl2/sdl2_osx.mm )
	ELSE()
		SET( SOURCES ${COMMON_SRC} ${MACOSX_SRC} )
	ENDIF()

	SET( MACOSX_PROJ_FILES
		sys/osx/English.lproj/ASLCore.nib/objects.xib
		sys/osx/English.lproj/InfoPlist.strings
		sys/osx/English.lproj/Localizable.strings
		sys/osx/English.lproj/MainMenu.nib/objects.nib
		sys/osx/French.lproj/ASLCore.nib/objects.xib
		sys/osx/French.lproj/InfoPlist.strings
		sys/osx/French.lproj/Localizable.strings
		sys/osx/French.lproj/MainMenu.nib/objects.nib
	)

	FOREACH( MACOSX_PROJ_FILE ${MACOSX_PROJ_FILES} )
	  #remove filename from path
	  STRING( REGEX REPLACE "sys/osx/(.*)/(.*)$" "Resources/\\1" FILE_OUTPUT_PATH "${MACOSX_PROJ_FILE}")
	  SET_SOURCE_FILES_PROPERTIES( ${MACOSX_PROJ_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION ${FILE_OUTPUT_PATH} )
	ENDFOREACH()

	SET( MACOSX_ICON_SOURCE sys/osx/Doom3.icns )
	SET_SOURCE_FILES_PROPERTIES( ${MACOSX_ICON_SOURCE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	SET( MACOSX_RSRC_SOURCE sys/osx/Doom\ 3.rsrc )
	SET_SOURCE_FILES_PROPERTIES( ${MACOSX_RSRC_SOURCE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	ADD_EXECUTABLE( "Doom 3" MACOSX_BUNDLE ${SOURCES} ${MACOSX_PROJ_FILES} ${MACOSX_ICON_SOURCE} ${MACOSX_RSRC_SOURCE} )
	TARGET_LINK_LIBRARIES( "Doom 3" ${FRAMEWORKS} curl idlib sound iconv )
	IF( SDL2_DIR )
		target_link_libraries( "Doom 3" ${SDL2_LIBRARIES})
	ENDIF()
	set_target_properties( "Doom 3" PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/sys/osx/Info.plist)
	#SET_TARGET_PROPERTIES( "Doom 3" PROPERTIES OUTPUT_NAME "Doom 3" )
ELSE()
	ADD_DEFINITIONS( ${CPP_WARNING_OPTIONS} )

	IF( SDL2_DIR )
		SET( SOURCES ${COMMON_SRC} ${SDL2_SRC} )
	ELSE()
		SET( SOURCES ${COMMON_SRC} ${WIN32_SRC} )
	ENDIF()
	ADD_EXECUTABLE(Doom3 WIN32 ${SOURCES} )
	LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/openal/lib )
	IF( SDL2_DIR )
		TARGET_LINK_LIBRARIES( Doom3 curl idlib sound opengl32 dsound dxguid dinput8 eaxguid user32 gdi32 comdlg32 winmm iphlpapi dbghelp ${SDL2_LIBRARIES} )
	ELSE()
		TARGET_LINK_LIBRARIES( Doom3 curl idlib sound opengl32 glu32 dsound dxguid dinput8 eaxguid user32 gdi32 comdlg32 winmm iphlpapi dbghelp )
	ENDIF()
	SET_TARGET_PROPERTIES( Doom3 PROPERTIES LINK_FLAGS "/LIBPATH:${CMAKE_CURRENT_SOURCE_DIR}\\openal\\lib /LIBPATH:${CMAKE_CURRENT_SOURCE_DIR}\\sys\\win32" )
ENDIF()

SOURCE_GROUP( "Sound" FILES
	${SOUND_SRC}
)

SOURCE_GROUP( "Framework" FILES
	${FRAMEWORK_SRC}
)

SOURCE_GROUP( "UI" FILES
	${UI_SRC}
)

SOURCE_GROUP( "CM" FILES
	${CM_SRC}
)

SOURCE_GROUP( "Renderer" FILES
	${RENDERER_SRC}
)

SOURCE_GROUP( "Tools" FILES
	${GAMETOOLS_SRC}
	${TOOLS_SRC}
)
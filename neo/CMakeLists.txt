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
set( CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin )
set( CMAKE_OSX_DEPLOYMENT_TARGET 10.9 )

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
	ADD_DEFINITIONS( -D_WIN32 -DDEBUG -D_CRT_SECURE_NO_WARNINGS -DID_DEDICATED )
ENDIF()

find_package(OpenGL)

add_subdirectory(idlib)
add_subdirectory(game)
add_subdirectory(d3xp)
add_subdirectory(sound)

SET( COMMON_SRC 
	./sys/sys_local.cpp
	./sound/snd_cache.cpp
	./sound/snd_decoder.cpp
	./sound/snd_efxfile.cpp
	./sound/snd_emitter.cpp
	./sound/snd_shader.cpp
	./sound/snd_system.cpp
	./sound/snd_wavefile.cpp
	./sound/snd_world.cpp
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
	./cm/CollisionModel_contacts.cpp
	./cm/CollisionModel_contents.cpp
	./cm/CollisionModel_debug.cpp
	./cm/CollisionModel_files.cpp
	./cm/CollisionModel_load.cpp
	./cm/CollisionModel_rotate.cpp
	./cm/CollisionModel_trace.cpp
	./cm/CollisionModel_translate.cpp
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

ADD_DEFINITIONS( -DID_ENFORCE_KEY=0 -D__DOOM_DLL__ -DID_ENABLE_CURL=1 )

IF( APPLE )
	SET( FRAMEWORKS "-framework Carbon -framework Cocoa -framework OpenGL -framework OpenAL -framework IOKit -framework CoreAudio -framework CoreGraphics -framework Foundation" )
	SET( SOURCES ${COMMON_SRC} ${MACOSX_SRC} )

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
	TARGET_LINK_LIBRARIES( "Doom 3" ${FRAMEWORKS} curl idlib sound )
	set_target_properties( "Doom 3" PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/sys/osx/Info.plist)
	#SET_TARGET_PROPERTIES( "Doom 3" PROPERTIES OUTPUT_NAME "Doom 3" )
ELSE()
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
	SET( SOURCES ${COMMON_SRC} ${WIN32_SRC} )
	ADD_EXECUTABLE(Doom3 WIN32 ${SOURCES} )
	LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/openal/lib )
	TARGET_LINK_LIBRARIES( Doom3 curl idlib sound opengl32 dsound dxguid dinput8 eaxguid dxerr user32 gdi32 comdlg32 winmm iphlpapi dbghelp )
	SET_TARGET_PROPERTIES( Doom3 PROPERTIES LINK_FLAGS "/LIBPATH:${CMAKE_CURRENT_SOURCE_DIR}\\openal\\lib /LIBPATH:${CMAKE_CURRENT_SOURCE_DIR}\\sys\\win32" )
ENDIF()


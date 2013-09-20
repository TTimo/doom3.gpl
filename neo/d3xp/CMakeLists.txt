FILE( GLOB D3XP_HDR *.h )
SET( BUILD_SHARED_LIBS ON )
INCLUDE_DIRECTORIES( )
ADD_DEFINITIONS( -D_D3XP -DCTF -DGAME_DLL -D__WITH_PB__ )

SET( D3XP_CPP
	./AF.cpp
	./AFEntity.cpp
	./Actor.cpp
	./BrittleFracture.cpp
	./Camera.cpp
	./Entity.cpp
	./Fx.cpp
	./GameEdit.cpp
	./Game_local.cpp
	./Game_network.cpp
	./Grabber.cpp
	./IK.cpp
	./Item.cpp
	./Light.cpp
	./Misc.cpp
	./Moveable.cpp
	./Mover.cpp
	./MultiplayerGame.cpp
	./Player.cpp
	./PlayerIcon.cpp
	./PlayerView.cpp
	./Projectile.cpp
	./Pvs.cpp
	./SecurityCamera.cpp
	./SmokeParticles.cpp
	./Sound.cpp
	./Target.cpp
	./Trigger.cpp
	./Weapon.cpp
	./WorldSpawn.cpp
	./ai/AAS.cpp
	./ai/AAS_debug.cpp
	./ai/AAS_pathing.cpp
	./ai/AAS_routing.cpp
	./ai/AI.cpp
	./ai/AI_Vagary.cpp
	./ai/AI_events.cpp
	./ai/AI_pathing.cpp
	./anim/Anim.cpp
	./anim/Anim_Blend.cpp
	./anim/Anim_Import.cpp
	./anim/Anim_Testmodel.cpp
	./gamesys/Class.cpp
	./gamesys/DebugGraph.cpp
	./gamesys/Event.cpp
	./gamesys/SaveGame.cpp
	./gamesys/SysCmds.cpp
	./gamesys/SysCvar.cpp
	./gamesys/TypeInfo.cpp
	./physics/Clip.cpp
	./physics/Force.cpp
	./physics/Force_Constant.cpp
	./physics/Force_Drag.cpp
	./physics/Force_Field.cpp
	./physics/Force_Grab.cpp
	./physics/Force_Spring.cpp
	./physics/Physics.cpp
	./physics/Physics_AF.cpp
	./physics/Physics_Actor.cpp
	./physics/Physics_Base.cpp
	./physics/Physics_Monster.cpp
	./physics/Physics_Parametric.cpp
	./physics/Physics_Player.cpp
	./physics/Physics_RigidBody.cpp
	./physics/Physics_Static.cpp
	./physics/Physics_StaticMulti.cpp
	./physics/Push.cpp
	./script/Script_Compiler.cpp
	./script/Script_Interpreter.cpp
	./script/Script_Program.cpp
	./script/Script_Thread.cpp
)

SET( SOURCES
	${D3XP_CPP}
	${D3XP_HDR}
)

ADD_DEFINITIONS( ${CPP_WARNING_OPTIONS} )
ADD_LIBRARY( d3xp ${SOURCES} )
target_link_libraries( d3xp idlib )
set_target_properties( d3xp PROPERTIES PREFIX "")


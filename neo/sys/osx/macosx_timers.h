#ifdef OMNI_TIMER

#import <OmniTimer/OTTimerNode.h>

extern OTTimerNode *RootNode;
extern OTTimerNode   *FrameNode;
extern OTTimerNode     *UpdateScreenNode;
extern OTTimerNode       *SurfaceMeshNode;
extern OTTimerNode         *LerpMeshVertexesNode;
extern OTTimerNode           *LerpMeshVertexesNode1;
extern OTTimerNode           *LerpMeshVertexesNode2;
extern OTTimerNode             *VectorArrayNormalizeNode;

extern void InitializeTimers();
extern void PrintTimers();

#endif // OMNI_TIMER

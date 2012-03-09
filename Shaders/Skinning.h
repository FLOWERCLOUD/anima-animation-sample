#ifndef __SKINNING_H__
#define __SKINNING_H__


#ifndef MAX_SKINNING_VECTORS
#define MAX_SKINNING_VECTORS 66*3
#endif

// bone transform data
float4 BoneTransforms[MAX_SKINNING_VECTORS] : BONE_TRANSFORMS;

#include "LinearBlendSkinning.h"
#include "DualQuaternionSkinning.h"
#include "TangentFrame.h"

#endif
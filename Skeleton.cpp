#include "Skeleton.h"
#include <cassert>
#include <set>
#include <map>
#include "aiScene.h"
#include "Debug.h"
#include "math.h"

using namespace std;

const char* SkeletonTraits<Math::Matrix3x4>::ShaderTechnique = "LinearBlendSkinning";
const char* SkeletonTraits<Math::DualQuaternion>::ShaderTechnique = "DualQuaternionSkinning";
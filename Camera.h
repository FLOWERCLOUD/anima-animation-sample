#pragma once

#include "math.h"

class Input;
class RenderContext;

class Camera
{
	Math::Vector mPosition;
	float mAzimuth;
	float mZenith;

	POINT mLastCursorPos;
	Math::Matrix mProjection;
	Math::Matrix mView;

	const Input& mInput;

public:
	Camera( const Input& input, const RenderContext& renderContext );
	~Camera();

	void update( float dt );
	Math::Matrix ViewMatrix() { return mView; }
	Math::Matrix ProjectionMatrix() { return mProjection; }
};


#pragma once
class TestEnvironment
{
	int mNumCubes;
	float mCubeRotation;
	float mCubeScale;

	bool mIsValid;

	int mRendererVersion;
	int mRandomSeed;

public:
	TestEnvironment( int argc, const LPWSTR* argv );
	~TestEnvironment(void);

	int GetNumCubes() const { return mNumCubes; }
	float GetCubeRotation() const { return mCubeRotation; }
	float GetCubeScale() const { return mCubeScale; }
	int GetRendererVersion() const { return mRendererVersion; }
	int GetRandomSeed() const { return mRandomSeed; }

	bool IsValid() const { return mIsValid; }
};


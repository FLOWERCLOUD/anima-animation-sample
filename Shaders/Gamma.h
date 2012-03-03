#ifndef __GAMMA_H__
#define __GAMMA_H__

float linearToSrgb( float value )
{ 
	return value < 0.0031308f ? 
		value * 12.92f :
		1.055f * pow( value, 1.0f/2.4f ) - 0.055f;
}

float4 linearToSrgb( float4 color )
{
	return float4( linearToSrgb( color.r ), linearToSrgb( color.g ), linearToSrgb( color.b ), color.a );
}

float srgbToLinear( float value )
{
	return value < 0.04045f ?
        value / 12.92f :
        pow( (value + 0.055f)/1.055f, 2.4f );
}

float4 srgbToLinear( float4 color )
{
	return float4( srgbToLinear( color.r ), srgbToLinear( color.g ), srgbToLinear( color.b ), color.a );
}

#endif
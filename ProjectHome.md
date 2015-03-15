# Anima Animation Sample #

## Overview ##
This code sample is meant to demonstrate different animation techniques for rendering skinned geometry. It makes use of the [Open Asset Import Library](http://assimp.sourceforge.net/) to import a skinned model and then converts the model data to a DirectX 9 compatible format. The model is rendered on-screen using a simple hlsl shader. The camera can be moved freely with the arrow keys and mouse input.

## Features ##

  * Linear blend skinning with 4x3 matrices
  * Dual quaternion skinning
  * QTangents
  * Normal Mapping
  * Gamma correct rendering

## Controls ##

  * 'M' Key: change animation skinning method
  * 'Q' Key: quit the sample
  * Space Bar: Stop model rotation and animation
  * Arrow keys: move camera
  * Mouse movement: rotate camera

## Acknowledgements ##

Special thanks to Ryan Lewis for creating this awesome model and allowing me to use and redistribute it!

Thanks also to the creators of the [Open Asset Import Library](http://assimp.sourceforge.net/), it's really simple to use and hasn't failed me yet.

Check out my  [developer blog](http://dev.theomader.com) for implementation details!

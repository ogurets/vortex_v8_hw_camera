# vortex_v8_hw_camera
Userland CLI utility to capture frames from Vimicro vc088x SoC camera (Viewsonic Viewpad 10e)

Right now this is a quick and dirty example of squeezing image data from a very closed and proprietary VC088x chip. The official driver for it is not v4l-compatible and there is no source code for a libcamera library using it.

So if you're about to dig into this mess (or a similar mess from another brand), this code will help you. A little.

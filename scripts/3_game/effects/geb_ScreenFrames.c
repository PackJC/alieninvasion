// Looping read-outs drawn on a model's hidden selection 0, one texture per frame named <pattern> with %1 = 00, 01, ...:
// the plasma rifle's rear screen (geb_PlasmaRifle.c) and the specimen tank's console (geb_AlienTank.c). Each owner
// steps its own client-side timer and only animates while the camera is near.

// Shows the frame after `frame` on owner's hidden selection 0 and returns its number.
int geb_NextScreenFrame(Object owner, string texturePattern, int frameCount, int frame)
{
	frame = (frame + 1) % frameCount;
	owner.SetObjectTexture(0, string.Format(texturePattern, frame.ToStringLen(2)));
	return frame;
}

// The current camera (the player's own in first person) is within the range whose square is rangeSq.
bool geb_CameraWithin(Object owner, float rangeSq)
{
	return vector.DistanceSq(GetGame().GetCurrentCameraPosition(), owner.GetPosition()) < rangeSq;
}

// Looping read-outs drawn on a model's hidden selection (0 unless given), one texture per frame named <pattern> with
// %1 = 00, 01, ...: the plasma rifle's rear screen (geb_PlasmaRifle.c), the specimen tank's console (geb_AlienTank.c)
// and the flying saucer's displays (geb_UFO.c). Each owner steps its own client-side timer and only animates while
// the camera is near.

// Shows the frame after `frame` on owner's hidden selection and returns its number. EntityAI, not Object: that's
// where SetObjectTexture is declared (the tank is a House, which is one too).
int geb_NextScreenFrame(EntityAI owner, string texturePattern, int frameCount, int frame, int selection = 0)
{
	frame = (frame + 1) % frameCount;
	owner.SetObjectTexture(selection, string.Format(texturePattern, frame.ToStringLen(2)));
	return frame;
}

// The current camera (the player's own in first person) is within the range whose square is rangeSq.
bool geb_CameraWithin(Object owner, float rangeSq)
{
	return vector.DistanceSq(GetGame().GetCurrentCameraPosition(), owner.GetPosition()) < rangeSq;
}

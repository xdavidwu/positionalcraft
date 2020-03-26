package org.positionalcraft.jni;

public class Link {
	private native boolean initLink();

	public Link() throws Exception {
		if (!initLink()) {
			throw new Exception("Link initialization failed");
		}
	}

	public native void updateIdentity(String id);
	public native void updateContext(String ctx);
	public native void updateAvatar(double x, double y, double z);
	public native void updateAvatarFront(double x, double y, double z);
	public native void updateCamera(double x, double y, double z);
	public native void updateCameraFront(double x, double y, double z);
	public native void updateTick();
}

package org.positionalcraft;

import net.fabricmc.api.ModInitializer;
import net.fabricmc.fabric.api.event.world.WorldTickCallback;

import net.minecraft.client.MinecraftClient;
import net.minecraft.client.network.ClientPlayerEntity;
import net.minecraft.client.network.ServerInfo;
import net.minecraft.client.render.Camera;
import net.minecraft.client.world.ClientWorld;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.nio.channels.FileChannel;

import org.positionalcraft.jni.Link;

public class PositionalCraft implements ModInitializer {
	private Link link = null;
	private MinecraftClient client = null;
	private Camera camera = null;

	@Override
	public void onInitialize() {
		try {
			// todo maplibraryName?
			File tmp = File.createTempFile("liblink", ".so");
			tmp.deleteOnExit();
			InputStream in = PositionalCraft.class.getResourceAsStream("/liblink.so");
			FileOutputStream out = new FileOutputStream(tmp);
			byte[] buf = new byte[4096];
			int read;
			while ((read = in.read(buf)) > 0) {
				out.write(buf, 0, read);
			}
			in.close();
			out.close();
			System.load(tmp.getAbsolutePath());
			link = new Link();
			client = MinecraftClient.getInstance();
		} catch (Exception e) {
			e.printStackTrace();
			return;
		}

		WorldTickCallback.EVENT.register((world) -> {
			ClientPlayerEntity player = client.player;
			if (player == null) {
				return;
			}

			int playerDimen = player.dimension.getRawId();
			if (playerDimen != world.dimension.getType().getRawId()) {
				return;
			}

			link.updateIdentity(player.getEntityName());
			link.updateAvatar(player.lastRenderX, player.lastRenderY, -player.lastRenderZ);
			Vec3d rot = player.getRotationVec(1.0F);
			link.updateAvatarFront(rot.x, rot.y, -rot.z);

			if (camera == null) {
				camera = client.gameRenderer.getCamera();
			}
			Vec3d camPos = camera.getPos();
			link.updateCamera(camPos.x, camPos.y, -camPos.z);
			float f = camera.getPitch() * 0.017453292F;
			float g = -camera.getYaw() * 0.017453292F;
			float h = MathHelper.cos(g);
			float i = MathHelper.sin(g);
			float j = MathHelper.cos(f);
			float k = MathHelper.sin(f);
			link.updateCameraFront((double)(i * j), (double)(-k), -(double)(h * j));

			ServerInfo server = client.getCurrentServerEntry();
			if (server != null) {
				// for multiplayer, label should be server description,
				// which should be always the same for the same instance
				link.updateContext(server.label + playerDimen);
			} else {
				link.updateContext("todo" + playerDimen);
			}
			link.updateTick();
		});
	}
}

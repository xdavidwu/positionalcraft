#include <jni.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif


struct LinkedMem {
#ifdef _WIN32
	UINT32	uiVersion;
	DWORD	uiTick;
#else
	uint32_t uiVersion;
	uint32_t uiTick;
#endif
	float	fAvatarPosition[3];
	float	fAvatarFront[3];
	float	fAvatarTop[3];
	wchar_t	name[256];
	float	fCameraPosition[3];
	float	fCameraFront[3];
	float	fCameraTop[3];
	wchar_t	identity[256];
#ifdef _WIN32
	UINT32	context_len;
#else
	uint32_t context_len;
#endif
	unsigned char context[256];
	wchar_t description[2048];
};

struct LinkedMem *lm = NULL;

/* if we have not increased tick for 5 seconds,
 * link will be lost, variables will be reset.
 */
static void maintain_link() {
	if (lm->uiVersion != 2) {
		wcscpy(lm->name, L"PositionalCraft");
		wcscpy(lm->description, L"A Minecraft Mumble Link plugin.");
		lm->uiVersion = 2;
	}
}

JNIEXPORT jboolean JNICALL Java_org_positionalcraft_jni_Link_initLink(JNIEnv *env, jobject obj) {
#ifdef _WIN32
	HANDLE hMapObject = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");

	if (hMapObject == NULL) {
		return JNI_FALSE;
	}

	lm = MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));

	if (lm == NULL) {
		CloseHandle(hMapObject);
		hMapObject = NULL;
		return JNI_FALSE;
	}

	return JNI_TRUE;
#else
	char memname[32];
	snprintf(memname, 32, "/MumbleLink.%d", getuid());

	int shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);

	if (shmfd < 0) {
		return JNI_FALSE;
	}

	lm = mmap(NULL, sizeof(struct LinkedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	if (lm == MAP_FAILED) {
		lm = NULL;
		return JNI_FALSE;
	}

	return JNI_TRUE;
#endif
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateTick(JNIEnv *env, jobject obj) {
	maintain_link();
	lm->uiTick++;
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateAvatar(JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z) {
	maintain_link();
	lm->fAvatarPosition[0] = x;
	lm->fAvatarPosition[1] = y;
	lm->fAvatarPosition[2] = z;
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateAvatarFront(JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z) {
	maintain_link();
	lm->fAvatarFront[0] = x;
	lm->fAvatarFront[1] = y;
	lm->fAvatarFront[2] = z;
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateCamera(JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z) {
	maintain_link();
	lm->fCameraPosition[0] = x;
	lm->fCameraPosition[1] = y;
	lm->fCameraPosition[2] = z;
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateCameraFront(JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z) {
	maintain_link();
	lm->fCameraFront[0] = x;
	lm->fCameraFront[1] = y;
	lm->fCameraFront[2] = z;
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateContext(JNIEnv *env, jobject obj, jstring ctx) {
	maintain_link();
	const char *str = (*env)->GetStringUTFChars(env, ctx, NULL);
	strncpy(lm->context, str, 256);
	(*env)->ReleaseStringUTFChars(env, ctx, str);
}

JNIEXPORT void JNICALL Java_org_positionalcraft_jni_Link_updateIdentity(JNIEnv *env, jobject obj, jstring id) {
	maintain_link();
	const char *str = (*env)->GetStringUTFChars(env, id, NULL);
	mbstowcs(lm->identity, str, 256);
	(*env)->ReleaseStringUTFChars(env, id, str);
}

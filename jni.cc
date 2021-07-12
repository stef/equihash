#include <jni.h>

extern "C" {

#include "equihash.h"

JNIEXPORT jbyteArray JNICALL Java_org_hsbp_equihash_Equihash_solve(JNIEnv *env,
		jobject ignore, jint n, jint k, jbyteArray seed) {
	jbyte* bufferPtrSeed = env->GetByteArrayElements(seed, NULL);
	const jsize seedLen = env->GetArrayLength(seed);

	const size_t csolLen = solsize(n, k);
	jbyteArray csol = env->NewByteArray(csolLen);
	jbyte* bufferPtrCSol = env->GetByteArrayElements(csol, NULL);

	int result = solve(n, k, (const uint8_t*)bufferPtrSeed, seedLen,
			(uint8_t*)bufferPtrCSol, csolLen);

	env->ReleaseByteArrayElements(csol, bufferPtrCSol, result ? 0 : JNI_ABORT);
	env->ReleaseByteArrayElements(seed, bufferPtrSeed, JNI_ABORT);

	return result ? csol : NULL;
}

JNIEXPORT jboolean JNICALL Java_org_hsbp_equihash_Equihash_verify(JNIEnv *env,
		jobject ignore, jint n, jint k, jbyteArray seed, jbyteArray sol) {
	jbyte* bufferPtrSeed = env->GetByteArrayElements(seed, NULL);
	jsize seedLen = env->GetArrayLength(seed);

	jbyte* bufferPtrSol  = env->GetByteArrayElements( sol, NULL);
	jsize  solLen = env->GetArrayLength( sol);

	int result = verify(n, k, (const uint8_t*)bufferPtrSeed, seedLen,
			(const uint8_t*)bufferPtrSol, solLen);

	env->ReleaseByteArrayElements( sol, bufferPtrSol , JNI_ABORT);
	env->ReleaseByteArrayElements(seed, bufferPtrSeed, JNI_ABORT);

	return result == 1 ? JNI_TRUE : JNI_FALSE;
}

}

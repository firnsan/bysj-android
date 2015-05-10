#include <string.h>
#include <jni.h>

#include <android/log.h>
#include <android/bitmap.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "rrimagelib.h"

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

JNIEXPORT jstring Java_com_example_bysj_NDKJpeg_stringFromJNI(JNIEnv* env, jobject thiz)
{
    return (*env)->NewStringUTF(env, "Hello from NDK !");
}

JNIEXPORT jboolean Java_com_example_bysj_NDKJpeg_getBitmap(JNIEnv* env,jobject thiz,
		jobject bitmap,jbyteArray dataarr,jint datalen)
{
    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    #define BYTE unsigned char
    BYTE *data = (BYTE *) (*env)->GetByteArrayElements(env,dataarr,0);//arraySrc is jbyteArray
    int dl = datalen; 

    //LOGD("width = %d; height = %d;", img->width, img->height);
    //LOGD("channels = %d",img->channels);
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGB_8888 !");
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }
    
    read_jpeg_mem(data,datalen,pixels);
    AndroidBitmap_unlockPixels(env, bitmap);
        
   LOGD("fill the bitmap");

   (*env)->ReleaseByteArrayElements(env, dataarr, data, 0);

    //free(img->pixels);
    //img->pixels = NULL;
    //free(img);
    //img = NULL;
    //LOGD("release the img memory");
}


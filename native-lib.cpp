#include <jni.h>
#include <string>
#include <cmath>
#include <algorithm>

// Функція для змішування двох кольорів (ARGB)
int blendColors(int color1, int color2, float ratio) {
    float inverse_ratio = 1.0f - ratio;
    
    int a = (int)((color1 >> 24 & 0xFF) * inverse_ratio + (color2 >> 24 & 0xFF) * ratio);
    int r = (int)((color1 >> 16 & 0xFF) * inverse_ratio + (color2 >> 16 & 0xFF) * ratio);
    int g = (int)((color1 >> 8 & 0xFF) * inverse_ratio + (color2 >> 8 & 0xFF) * ratio);
    int b = (int)((color1 & 0xFF) * inverse_ratio + (color2 & 0xFF) * ratio);
    
    return (a << 24) | (r << 16) | (g << 8) | b;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_unicreate_NativeHelper_applySmudge(
        JNIEnv* env, jclass clazz, 
        jintArray pixels_array, 
        jint width, jint height, 
        jint startX, jint startY, 
        jint endX, jint endY, 
        jint radius, jfloat strength) {
    
    // Отримуємо доступ до масиву пікселів
    jint* pixels = env->GetIntArrayElements(pixels_array, nullptr);
    if (pixels == nullptr) return;

    int dx = endX - startX;
    int dy = endY - startY;
    
    // Якщо рух занадто малий, нічого не робимо
    if (dx == 0 && dy == 0) {
        env->ReleaseIntArrayElements(pixels_array, pixels, JNI_ABORT);
        return;
    }

    // Проходимо по колу навколо поточного пальця
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                int currentX = endX + x;
                int currentY = endY + y;

                int sourceX = startX + x;
                int sourceY = startY + y;

                // Перевіряємо, чи не виходимо за межі полотна
                if (currentX >= 0 && currentX < width && currentY >= 0 && currentY < height &&
                    sourceX >= 0 && sourceX < width && sourceY >= 0 && sourceY < height) {
                    
                    int dstIndex = currentY * width + currentX;
                    int srcIndex = sourceY * width + sourceX;

                    int srcColor = pixels[srcIndex];
                    int dstColor = pixels[dstIndex];

                    // Змішуємо колір джерела з кольором під пальцем
                    pixels[dstIndex] = blendColors(dstColor, srcColor, strength);
                }
            }
        }
    }

    // Записуємо зміни назад у Java
    env->ReleaseIntArrayElements(pixels_array, pixels, 0);
}

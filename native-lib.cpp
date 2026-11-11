#include <jni.h>
#include <cmath>

extern "C" JNIEXPORT void JNICALL
Java_com_example_unicreate_NativeHelper_applySmudge(
        JNIEnv* env, jclass clazz, 
        jintArray smudge_pixels, 
        jint diameter,
        jint canvasW, jint canvasH,
        jfloat strength) {
    
    jint* pixels = env->GetIntArrayElements(smudge_pixels, nullptr);
    if (pixels == nullptr) return;

    jint* resultPixels = new jint[diameter * diameter];

    float cx = diameter / 2.0f;
    float cy = diameter / 2.0f;
    float maxR = diameter / 2.0f;

    // Збільшуємо радіус розмиття для кращого ефекту (було 1, стало 3)
    int blurRadius = 3; 

    for (int y = 0; y < diameter; y++) {
        for (int x = 0; x < diameter; x++) {
            int idx = y * diameter + x;
            
            // Створюємо круглу м'яку маску
            float dx_center = x - cx;
            float dy_center = y - cy;
            float dist = std::sqrt(dx_center * dx_center + dy_center * dy_center);
            float alphaMask = 1.0f - (dist / maxR);
            if (alphaMask < 0) alphaMask = 0;

            if (alphaMask == 0.0f) {
                resultPixels[idx] = 0;
                continue;
            }

            int r = 0, g = 0, b = 0, a = 0;
            int count = 0;

            // Розмиття
            for (int dy = -blurRadius; dy <= blurRadius; dy++) {
                for (int dx = -blurRadius; dx <= blurRadius; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    
                    if (nx >= 0 && nx < diameter && ny >= 0 && ny < diameter) {
                        int c = pixels[ny * diameter + nx];
                        a += (c >> 24) & 0xFF;
                        r += (c >> 16) & 0xFF;
                        g += (c >> 8) & 0xFF;
                        b += c & 0xFF;
                        count++;
                    }
                }
            }

            if (count == 0) {
                resultPixels[idx] = 0;
                continue;
            }

            int avgA = a / count;
            int avgR = r / count;
            int avgG = g / count;
            int avgB = b / count;

            int origColor = pixels[idx];
            int origA = (origColor >> 24) & 0xFF;
            int origR = (origColor >> 16) & 0xFF;
            int origG = (origColor >> 8) & 0xFF;
            int origB = origColor & 0xFF;

            int finalA = (int)(origA * (1.0f - strength) + avgA * strength);
            int finalR = (int)(origR * (1.0f - strength) + avgR * strength);
            int finalG = (int)(origG * (1.0f - strength) + avgG * strength);
            int finalB = (int)(origB * (1.0f - strength) + avgB * strength);

            // Застосовуємо маску до прозорості (краї стануть прозорими)
            finalA = (int)(finalA * alphaMask);

            resultPixels[idx] = (finalA << 24) | (finalR << 16) | (finalG << 8) | finalB;
        }
    }

    for (int i = 0; i < diameter * diameter; i++) {
        pixels[i] = resultPixels[i];
    }

    delete[] resultPixels;
    env->ReleaseIntArrayElements(smudge_pixels, pixels, 0);
}

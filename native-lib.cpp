#include <jni.h>
#include <cmath>
#include <algorithm>

extern "C" JNIEXPORT void JNICALL
Java_com_example_unicreate_NativeHelper_applySmudge(
        JNIEnv* env, jclass clazz, 
        jintArray smudge_pixels, 
        jint diameter,
        jint canvasW, jint canvasH,
        jfloat strength,
        jint dx, jint dy) {
    
    // Отримуємо доступ до масиву пікселів з Java
    jint* pixels = env->GetIntArrayElements(smudge_pixels, nullptr);
    if (pixels == nullptr) return;

    // Створюємо тимчасовий масив для результату
    jint* resultPixels = new jint[diameter * diameter];
    int radius = diameter / 2;

    // Обмежуємо зсув, щоб не вилізти за межі нашого маленького квадрата (пензля)
    int maxShift = radius / 2;
    int shiftX = std::max(-maxShift, std::min(maxShift, dx));
    int shiftY = std::max(-maxShift, std::min(maxShift, dy));

    for (int y = 0; y < diameter; y++) {
        for (int x = 0; x < diameter; x++) {
            int idx = y * diameter + x;
            
            int origColor = pixels[idx];
            int origA = (origColor >> 24) & 0xFF;
            
            // Якщо піксель повністю прозорий, не чіпаємо його (щоб не було чорних плям на фоні)
            if (origA == 0) {
                resultPixels[idx] = 0;
                continue;
            }

            // Звідки "тягнемо" колір (попередня позиція пальця)
            int srcX = x - shiftX;
            int srcY = y - shiftY;
            
            int srcColor = origColor;
            int srcA = origA;

            // Якщо джерело всередині пензля, беремо колір звідти
            if (srcX >= 0 && srcX < diameter && srcY >= 0 && srcY < diameter) {
                int c = pixels[srcY * diameter + srcX];
                int ca = (c >> 24) & 0xFF;
                if (ca > 0) {
                    srcColor = c;
                    srcA = ca;
                }
            }

            // Змішуємо поточний піксель з тим, що притягнули (залежно від strength)
            int r = (int)(((origColor >> 16) & 0xFF) * (1.0f - strength) + ((srcColor >> 16) & 0xFF) * strength);
            int g = (int)(((origColor >> 8) & 0xFF) * (1.0f - strength) + ((srcColor >> 8) & 0xFF) * strength);
            int b = (int)((origColor & 0xFF) * (1.0f - strength) + (srcColor & 0xFF) * strength);
            int a = (int)(origA * (1.0f - strength) + srcA * strength);

            // Кругла маска для м'яких країв пензля (щоб не було квадратів)
            float dx_center = x - radius;
            float dy_center = y - radius;
            float dist = std::sqrt(dx_center * dx_center + dy_center * dy_center);
            float alphaMask = 1.0f - (dist / radius);
            if (alphaMask < 0) alphaMask = 0;

            // Робимо краї ще більш плавними (Smoothstep)
            alphaMask = alphaMask * alphaMask * (3.0f - 2.0f * alphaMask);

            a = (int)(a * alphaMask);
            if (a < 10) a = 0; // Відсікаємо майже невидимий шум по краях

            resultPixels[idx] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    // Копіюємо результат назад у масив, який прийшов з Java
    for (int i = 0; i < diameter * diameter; i++) {
        pixels[i] = resultPixels[i];
    }

    // Звільняємо пам'ять
    delete[] resultPixels;
    
    // Повертаємо масив у Java
    env->ReleaseIntArrayElements(smudge_pixels, pixels, 0);
}

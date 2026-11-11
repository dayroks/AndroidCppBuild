#include <jni.h>
#include <cmath>

extern "C" JNIEXPORT void JNICALL
Java_com_example_unicreate_NativeHelper_applySmudge(
        JNIEnv* env, jclass clazz, 
        jintArray smudge_pixels, 
        jint diameter,
        jint canvasW, jint canvasH,
        jfloat strength) {
    
    // Отримуємо доступ до масиву пікселів
    jint* pixels = env->GetIntArrayElements(smudge_pixels, nullptr);
    if (pixels == nullptr) return;

    // Створюємо тимчасовий масив для результату, щоб не псувати сусідні пікселі під час розрахунку
    jint* resultPixels = new jint[diameter * diameter];

    // Проходимо по кожному пікселю в маленькому квадраті (діаметрі)
    for (int y = 0; y < diameter; y++) {
        for (int x = 0; x < diameter; x++) {
            int idx = y * diameter + x;
            
            int r = 0, g = 0, b = 0, a = 0;
            int count = 0;

            // Беремо сусідні пікселі (радіус 1 піксель, тобто 3x3)
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
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

            // Знаходимо середнє значення кольору
            int avgA = a / count;
            int avgR = r / count;
            int avgG = g / count;
            int avgB = b / count;

            // Змішуємо оригінальний колір з розмитим (залежно від strength)
            int origColor = pixels[idx];
            int origA = (origColor >> 24) & 0xFF;
            int origR = (origColor >> 16) & 0xFF;
            int origG = (origColor >> 8) & 0xFF;
            int origB = origColor & 0xFF;

            int finalA = (int)(origA * (1.0f - strength) + avgA * strength);
            int finalR = (int)(origR * (1.0f - strength) + avgR * strength);
            int finalG = (int)(origG * (1.0f - strength) + avgG * strength);
            int finalB = (int)(origB * (1.0f - strength) + avgB * strength);

            // Записуємо в тимчасовий масив
            resultPixels[idx] = (finalA << 24) | (finalR << 16) | (finalG << 8) | finalB;
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

extern __inline__ uint64_t curr_ticks() {
    uint64_t x;
    __asm__ volatile ("rdtsc\n\tshl $32, %%rdx\n\tor %%rdx, %%rax" : "=a" (x) : : "rdx");
    return x;
}

int my_abs(int i)
{
    if (i == 0x80000000)
    {
        return 0x77777777;
    }

    if (i < 0)
    {
        return -i;
    }

    return i;
}

double exps[62];

double my_exp(double x)
{
    union
    {
        double d;
        __int64_t i64;
        __int32_t i32[2];
    } t = { .d = x };

    __int64_t p = ((t.i32[1] >> 20) & 0x7FF) - 1023;
    __int64_t m = (t.i64 & (__int64_t) (0x000FFFFFFFFFFFFF)) | (__int64_t) (0x0010000000000000);
    // printf("p = %ld %p\nm = %ld %p\n", p, (void *) p, m, (void *) m);

    if (p < 0) {
        m >>= -p;
    } else {
        m <<= p;
    }

    // printf("m = %ld %\n", m, (void *) m);

    __int64_t b = 1;
    double r = 1;
    for(int i = 0; i < 62; i++, b <<= 1) {
        if (b & m) {
            r *= exps[i] * (b & m > 0);
        }
    }

    // разбить 8 байт на 8 массив каждый по 256 значений (комбинации произведений)
    // 52 - 45 + 1
    // 256 перебор, расчитываем
    // 10^(-44)
    // i * 2^(-52) <- чтобы заполнить первую таблицу
    // i * 2^(-44) <- для второй таблицы
    // важно в union использовать uint8
    // сделать дз, потом показать на экзамене

    return r;
}

int main(void)
{
    // float af1[50], af2[50];

    // for (int i = 0; i < 50; i++) {
    //     af1[i] = 1000 * i;
    //     af2[i] = 0.033;
    // }

    // float s1 = 0, s2 = 0;

    // for (int i = 0; i < 50; i++) {
    //     s1 += af1[i];
    //     s2 += af2[i];
    // }

    // for (int i = 0; i < 50; i++) {
    //     s1 += af2[i];
    //     s2 += af1[i];
    // }

    // float - значащих десятичных цифр примерно 7-8, double - 16-17
    // параллельное прогр - однопроцессный алгоритм на несколько потоков или процессов, результат может отличаться
    // 0 <= x <= 1

    // float s = 1, sp = 0;
    // int x = 1, f = 1;
    // while (s - sp > 0) {
    //     sp = s;
    //     s += 1/f;
    //     f *= x;
    //     x++;
    // }
    // printf("%f %d\n", s, x);

    // double d = 1;
    // double s = 1, sp = 0;
    // double x = 709.789; // 708   1,23 10^306

    // int i = 1;

    // while (sp != s) {
    //     sp = s;
    //     d = (d / i) * x;
    //     s += d;
    //     i++;
    // }

    // printf("%f %d\n", s, i);

    // 709^708/708! = 1
    // 200! > 10^306
    // x < 170 -- то можно считать

    // e^(a + b) = e^a e^b
    // e^a

    // double x = 1;
    // int i = 0;

    // for (; exp(x) != 1; i++)
    // {
    //     printf("%lf %d\n", x, i);
    //     x /= 2;
    // }

    // printf("%d %lf\n", i, x);

    // 709 -> 10 разрядов
    // 52 разрядов
    // создать массив m { 62 } элемента
    // m[0] = exp(2^-52)
    // ...
    // m[61] = exp(2^9) <- заполнять с конца.

    // x -> преобразовать в 62 разряда
    // x -> 53 разряда + мантисса
    // max_x = 709
    // x < 2^52 => 0
    // int64_<= достать 52 разряда из float
    // int64
    // 52 разряда => целое число как сдвинутое на 52 разряда
    // 32

    // double x = 10;
    // union {
    //     double d;
    //     __int64_t i64;
    //     __int32_t i32[2];
    // } t = x;

    // 1) порядок в старших разрядах (11 бит)
    // (t.i32[1] >> 20) & 0x7FF) - 1023 // правильный порядок 2^(+-)
    // 2)
    // __int64_t = t.i64 & __int64_t(0x000FFFFFFFFFFFFF) | __in64_t(0x0010000000000000);
    // 3) если порядок положительный, то свдигаем вправо иначе влево -- ???
    // 4) цикл от 0 до 61 проверяем биты если i-ый бит == 1, то прибавляю. 61 --
    // 5)

    double e = 1 << 9;
    for (int i = 61; i >= 0; i--)
    {
        exps[i] = exp(e);
        e /= 2;
    }

    // for (int i = 0; i < 1000; i++) {
    //     double x = i / 10.0;
    //     double my_r = my_exp(x);
    //     double r = exp(x);
    //     if (abs(my_r - r) / r > 0.000001) {
    //         printf(" ------ %lf %lf %lf\n", r, my_r, x);
    //     }
    // }


    uint64_t start_tick = curr_ticks();
    double start_d = omp_get_wtime();
    for (int i = 0; i < 10000; i++) {
        double x = i / 10.0;
        my_exp(x);
    }
    double end_d = omp_get_wtime();
    uint64_t end_tick = curr_ticks();
    printf("my_exp = %lu \t%lg\n", end_tick - start_tick, end_d - start_d);

    start_tick = curr_ticks();
    start_d = omp_get_wtime();
    for (int i = 0; i < 10000; i++) {
        double x = i / 10.0;
        exp(x);
    }
    end_d = omp_get_wtime();

    end_tick = curr_ticks();
    printf("exp    = %lu \t%lg\n", end_tick - start_tick, end_d - start_d);

    return 0;
}

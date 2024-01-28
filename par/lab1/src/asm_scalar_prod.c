#include "asm_scalar_prod.h"

void dot_prod_vector_arrays_asm(double *res, const dvec *a, const dvec *b, int n)
{
    for (const dvec *a_end = a + n - 1; a < a_end; a += 2, b += 2)
    {
        asm(
            "vmovupd ymm0, %1\n"
            "vmovupd ymm1, %2\n"
            "vmovupd ymm2, %3\n"
            "vmovupd ymm3, %4\n"
            "vmulpd ymm0, ymm0, ymm1\n"
            "vmulpd ymm2, ymm2, ymm3\n"
            "vhaddpd ymm1, ymm0, ymm2\n"
            "vextractf128 xmm3, ymm1, 1\n"
            "vaddpd ymm0, ymm1, ymm3\n"
            "movupd xmmword ptr [%0], xmm0\n"
            :
            : "r"(res), "m"(*a), "m"(*b), "m"(*(a + 1)), "m"(*(b + 1))
            : "ymm0", "ymm1", "ymm2", "ymm3");
    }

    if (n & 1)
    {
        asm volatile(
            "vmovupd ymm0, %1\n"
            "vmovupd ymm1, %2\n"
            "vmovupd ymm2, %3\n"
            "vmovupd ymm3, %4\n"
            "vmulpd ymm0, ymm0, ymm1\n"
            "vmulpd ymm2, ymm2, ymm3\n"
            "vhaddpd ymm1, ymm0, ymm2\n"
            "vextractf128 xmm3, ymm1, 1\n"
            "vaddpd ymm0, ymm1, ymm3\n"
            "movlpd %0, xmm0\n"
            :
            : "m"(*res), "m"(*a), "m"(*b), "m"(*a), "m"(*b)
            : "ymm0", "ymm1", "ymm2", "ymm3", "memory");
    }
}

/*
a:
a11 a12 a13 a14
a21 a22 a23 a24
a31 a32 a33 a34
a41 a42 a43 a44

b:
b11 b12 b13 b14
b21 b22 b23 b24
b31 b32 b33 b34
b41 b42 b43 b44

a11 * b11 + a12 * b21


      0 1 2 3
ymm0: 0 1 2 3
ymm1: 4 5 6 7
ymm2: 8 9 a b
ymm3: c d e f

      0 1 2 3
ymm4: 0 4 8 c
ymm5: 1 5 9 d
ymm6: 2 6 a e
ymm7: 3 7 b f

vshufps ymm4, ymm0, ymm2, 0b00 00 00 00
vshufps ymm4, ymm1, ymm3, 0b00 00 00 00

044h
0eeh
*/

void mul_matrixes_asm(fmatrix *res, const fmatrix *a, const fmatrix *b, int n) {
    for (const fmatrix *a_end = a + n; a < a_end; a++, b++, res++) {
        fmatrix r;
        asm (
            "movaps xmm0, %4\n"
            "movaps xmm1, %5\n"
            "movaps xmm2, %6\n"
            "movaps xmm3, %7\n"

            // 0 1 4 5
            "vshufps xmm4, xmm0, xmm1, 0b01000100\n"
            // 0 4 1 5
            "shufps xmm4, xmm4, 0b11011000\n"

            // 2 3 6 7
            "vshufps xmm5, xmm0, xmm1, 0b11101110\n"
            // 2 6 3 7
            "shufps xmm5, xmm5, 0b11011000\n"

            // 8 9 c d
            "vshufps xmm6, xmm2, xmm3, 0b01000100\n"
            // 8 c 9 d
            "shufps xmm6, xmm6, 0b11011000\n"

            // a b e f
            "vshufps xmm7, xmm2, xmm3, 0b11101110\n"
            // a e b f
            "shufps xmm7, xmm7, 0b11011000\n"

            // 0 4 8 c
            "vshufps xmm0, xmm4, xmm6, 0b01000100\n"
            // 1 5 9 d
            "vshufps xmm1, xmm4, xmm6, 0b11101110\n"
            // 2 6 a e
            "vshufps xmm2, xmm5, xmm7, 0b01000100\n"
            // 3 7 b f
            "vshufps xmm3, xmm5, xmm7, 0b11101110\n"

            "movaps xmm4, %8\n"
            "movaps xmm5, %9\n"
            "movaps xmm6, %10\n"
            "movaps xmm7, %11\n"

            // 0 4 8 c
            "vmulps xmm11, xmm0, xmm4\n"
            // 1 5 9 d
            "vmulps xmm12, xmm1, xmm4\n"
            // 2 6 a e
            "vmulps xmm13, xmm2, xmm4\n"
            // 3 7 b f
            "vmulps xmm14, xmm3, xmm4\n"

            // 0+4 8+с 1+5 9+d
            "haddps xmm11, xmm12\n"
            // 2+6 a+e 3+7 b+f
            "haddps xmm13, xmm14\n"
            // 0+4+8+с 1+5+9+d 2+6+a+e 3+7+b+f
            "vhaddps xmm4, xmm11, xmm13\n"

            // 0 4 8 c
            "vmulps xmm11, xmm0, xmm5\n"
            // 1 5 9 d
            "vmulps xmm12, xmm1, xmm5\n"
            // 2 6 a e
            "vmulps xmm13, xmm2, xmm5\n"
            // 3 7 b f
            "vmulps xmm14, xmm3, xmm5\n"

            // 0+4 8+с 1+5 9+d
            "haddps xmm11, xmm12\n"
            // 2+6 a+e 3+7 b+f
            "haddps xmm13, xmm14\n"
            // 0+4+8+с 1+5+9+d 2+6+a+e 3+7+b+f
            "vhaddps xmm5, xmm11, xmm13\n"

            // 0 4 8 c
            "vmulps xmm11, xmm0, xmm6\n"
            // 1 5 9 d
            "vmulps xmm12, xmm1, xmm6\n"
            // 2 6 a e
            "vmulps xmm13, xmm2, xmm6\n"
            // 3 7 b f
            "vmulps xmm14, xmm3, xmm6\n"

            // 0+4 8+с 1+5 9+d
            "haddps xmm11, xmm12\n"
            // 2+6 a+e 3+7 b+f
            "haddps xmm13, xmm14\n"
            // 0+4+8+с 1+5+9+d 2+6+a+e 3+7+b+f
            "vhaddps xmm6, xmm11, xmm13\n"


            // 0 4 8 c
            "vmulps xmm11, xmm0, xmm7\n"
            // 1 5 9 d
            "vmulps xmm12, xmm1, xmm7\n"
            // 2 6 a e
            "vmulps xmm13, xmm2, xmm7\n"
            // 3 7 b f
            "vmulps xmm14, xmm3, xmm7\n"

            // 0+4 8+с 1+5 9+d
            "haddps xmm11, xmm12\n"
            // 2+6 a+e 3+7 b+f
            "haddps xmm13, xmm14\n"
            // 0+4+8+с 1+5+9+d 2+6+a+e 3+7+b+f
            "vhaddps xmm7, xmm11, xmm13\n"

            "movupd xmmword ptr [%0], xmm4\n"
            "movupd xmmword ptr [%1], xmm5\n"
            "movupd xmmword ptr [%2], xmm6\n"
            "movupd xmmword ptr [%3], xmm7\n"
            :
            : "r"(&(*res)[0]), "r"(&(*res)[1]), "r"(&(*res)[2]), "r"(&(*res)[3]),
              "m"((*b)[0]), "m"((*b)[1]), "m"((*b)[2]), "m"((*b)[3]),
              "m"((*a)[0]), "m"((*a)[1]), "m"((*a)[2]), "m"((*a)[3])
            : "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
    }
}


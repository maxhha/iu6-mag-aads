#include <stdio.h>

int iCodeGenRowsCols_V7(FILE *pf, int iFlagRows, int iFlagCols)
{
    const char *pcAdd_iCmin = "";
    const char *pcAdd_iCminOr0 = "";
    const char *pcCurr = "";
    const char *pcCorrSum = "";
    const char *pcAll = "";
    if (0 != iFlagRows)
    {
        pcCorrSum = " - u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1)";
        if (1 != iFlagCols)
            pcAll = "All";
    }
    fprintf(pf, "\n");
    if (0 == iFlagCols)
    {
        fprintf(pf, "        // Первые [0, 2*Win_cen[ колонки\n");
        fprintf(pf, "        for (iCol = 0; iCol < (Win_cen << 1); iCol++)\n");
        fprintf(pf, "        {\n");
        fprintf(pf, "            iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);\n");
        fprintf(pf, "            iCmax = iCol + Win_cen;\n");
        fprintf(pf, "\n");
    }
    else if (1 == iFlagCols)
    {
        pcAdd_iCmin = " + iCmin";
        pcCurr = "Curr";
        pcAdd_iCminOr0 = "0";
        fprintf(pf, "        // Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки\n");
        fprintf(pf, "        puiSumCurr = puiSum + Win_cen;\n");
        if (0 != iFlagRows)
        {
            fprintf(pf, "        pHist = pHistAll + (iCol - Win_cen);\n");
            fprintf(pf, "        for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)\n");
        }
        else
            fprintf(pf, "        for (; iCol <= iColMax; iCol++, puiSumCurr++)\n");
        fprintf(pf, "        {\n");
        if (0 == iFlagRows)
        {
            fprintf(pf, "            iCmin = iCol - Win_cen;\n");
            fprintf(pf, "\n");
        }
    }
    else
    {
        pcAdd_iCminOr0 = "iCmin";
        fprintf(pf, "        // Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки\n");
        fprintf(pf, "        for (; iCol < ar_cmmIn.m_i64W; iCol++)\n");
        fprintf(pf, "        {\n");
        fprintf(pf, "            iCmin = iCol - Win_cen;\n");
        fprintf(pf, "            iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);\n");
        fprintf(pf, "\n");
    }
    fprintf(pf, "            if ((d = (pRowIn[iCol] - pseudo_min)*kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!\n");
    if (0 == iFlagRows)
        fprintf(pf, "                pbBrightnessRow[iCol] = 0;\n");
    else
        fprintf(pf, "                u32ValueAdd = 0;\n");
    fprintf(pf, "            else\n");
    fprintf(pf, "            {\n");
    fprintf(pf, "                if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, "                {\n");
        fprintf(pf, "                     pbBrightnessRow[iCol] = u32ValueAdd = 255;\n");
        fprintf(pf, "                     pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W%s);\n", pcAdd_iCmin);
        fprintf(pf, "                }\n");
        fprintf(pf, "                else\n");
        fprintf(pf, "                {\n");
        fprintf(pf, "                     pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);\n");
        fprintf(pf, "                     pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W%s);\n",
                pcAdd_iCmin);
        fprintf(pf, "                }\n");
    }
    else
    {
        fprintf(pf, "                    u32ValueAdd = 255;\n");
        fprintf(pf, "            }\n");
        fprintf(pf, "            if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))\n");
        fprintf(pf, "            {\n");
        fprintf(pf, "                  pHistSub = pHist%s + (u32ValueSub * ar_cmmIn.m_i64W);\n", pcAll);
        fprintf(pf, "                  pHistAdd = pHist%s + (u32ValueAdd * ar_cmmIn.m_i64W);\n", pcAll);
        fprintf(pf, "                  pbBrightnessRow[iCol] = u32ValueAdd;\n");
        fprintf(pf, "\n");
    }
    fprintf(pf, "                // Добавляем яркость в Win_size гистограмм и сумм\n");
    if (1 != iFlagCols)
        fprintf(pf, "                for (i = iCmin; i <= iCmax; i++)\n");
    else
        fprintf(pf, " for (i = 0; i < Win_size; i++)\n");
    fprintf(pf, " puiSum%s[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1))%s;\n", pcCurr, pcCorrSum);
    fprintf(pf, " }\n");
    if (0 != iFlagCols)
    {
        if (0 == iFlagRows)
        {
            fprintf(pf, " if ((Win_size - 1) == iRow)\n");
            fprintf(pf, " {\n");
            fprintf(pf, " pfRowOut[0] = static_cast<float>(puiSum%s[%s] * Size_obratn);\n",
                    pcCurr, pcAdd_iCminOr0);
            fprintf(pf, " pfRowOut++;\n");
            fprintf(pf, " }\n");
        }
        else
        {
            fprintf(pf, " pfRowOut[0] = static_cast<float>(puiSum%s[%s] * Size_obratn);\n", pcCurr,
                    pcAdd_iCminOr0);
            fprintf(pf, " pfRowOut++;\n");
        }
    }
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGenRows_V7(FILE *pf, int iFlagRows)
{
    fprintf(pf, "\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, " // Первые [0, Win_size - 1] строки\n");
        fprintf(pf, " pbBrightnessRow = pbBrightness;\n");
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(Win_cen) + Win_cen;\n");
        fprintf(pf, " for (iRow = 0; iRow < Win_size; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)\n");
    }
    else
    {
        fprintf(pf, " // Последующие строки [Win_size, ar_cmmIn.m_i64H[\n");
        fprintf(pf, " for (iRow = Win_size; iRow < ar_cmmIn.m_i64H; iRow++)\n");
    }
    fprintf(pf, " {\n");
    fprintf(pf, " // Обнуляем края строк.\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);\n");
    fprintf(pf, "\n");
    fprintf(pf, " uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);\n");
    if (0 != iFlagRows)
    {
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(iRow - Win_cen) + Win_cen;\n");
        fprintf(pf, " iPos = (iRow - Win_size) %% Win_size;\n");
        fprintf(pf, " pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);\n");
    }
    // Первые колонки
    if (0 != iCodeGenRowsCols_V7(pf, iFlagRows, 0))
        return 10;
    // Средние колонки
    if (0 != iCodeGenRowsCols_V7(pf, iFlagRows, 1))
        return 20;
    // Последние колонки
    if (0 != iCodeGenRowsCols_V7(pf, iFlagRows, 2))
        return 30;
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGen_V7(const char *a_pcFilePath)
{
    FILE *pf = fopen(a_pcFilePath, "wt");
    if (nullptr == pf)
        return 10;
    // Пролог
    fprintf(pf, "void TextureFiltr_Mean_V7(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)\n ");
    fprintf(pf, "{\n");
    fprintf(pf, " int Win_size = (Win_cen << 1) + 1;\n");
    fprintf(pf, " int Win_lin = Win_size * Win_size;\n");
    fprintf(pf, " double Size_obratn = 1.0 / Win_lin;\n");
    fprintf(pf, "\n");
    fprintf(pf, " // Кэш для гистограмм\n");
    fprintf(pf, " uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]\n");
    fprintf(pf, " memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);\n");
    fprintf(pf, " uint16_t *pHist, *pHistAdd, *pHistSub;\n");
    fprintf(pf, "\n");
    fprintf(pf, " // Кэш для сумм\n");
    fprintf(pf, " uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;\n");
    fprintf(pf, " memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);\n");
    fprintf(pf, "\n");
    fprintf(pf, " // Кэш для яркостей\n");
    fprintf(pf, " uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * Win_size];\n");
    fprintf(pf, " uint8_t *pbBrightnessRow;\n");
    fprintf(pf, "\n");
    fprintf(pf, " int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - Win_size;\n");
    fprintf(pf, " ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);\n");
    fprintf(pf, "\n");
    fprintf(pf, " // Обнуляем края. Первые строки.\n");
    fprintf(pf, " for (iRow = 0; iRow < Win_cen; iRow++)\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
    fprintf(pf, "\n");
    fprintf(pf, " int64_t i, iCmin, iCmax, iPos = 0;\n");
    fprintf(pf, " uint32_t u32ValueAdd, u32ValueSub;\n");
    fprintf(pf, " double d;\n");
    // Начальные строки
    if (0 != iCodeGenRows_V7(pf, 0))
    {
        fclose(pf);
        return 20;
    }
    // Последующие строки
    if (0 != iCodeGenRows_V7(pf, 1))
    {
        fclose(pf);
        return 30;
    }
    // Эпилог
    fprintf(pf, "\n");
    fprintf(pf, " // Обнуляем края. Последние строки.\n");
    fprintf(pf, " for (iRow = ar_cmmIn.m_i64H - Win_cen; iRow < ar_cmmOut.m_i64H; iRow++)\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
    fprintf(pf, "\n");
    fprintf(pf, " if (nullptr != pHistAll)\n");
    fprintf(pf, " delete[] pHistAll;\n");
    fprintf(pf, " if (nullptr != puiSum)\n");
    fprintf(pf, " delete[] puiSum;\n");
    fprintf(pf, " if (nullptr != pbBrightness)\n");
    fprintf(pf, " delete[] pbBrightness;\n");
    fprintf(pf, "}\n");
    fclose(pf);
    printf("FINISHED");
    return 0;
}

//================================================================
//================================================================

int iCodeGenRowsCols_V8(FILE *pf, int Win_cen, int iFlagRows, int iFlagCols)
{
    int Win_size = (Win_cen << 1) + 1;
    int Win_lin = Win_size * Win_size;

    const char *pcAdd_iCmin = "";
    const char *pcAdd_iCminOr0 = "";
    const char *pcCurr = "";
    const char *pcCorrSum = "";
    const char *pcAll = "";
    if (0 != iFlagRows)
    {
        pcCorrSum = " - u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1)";
        if (1 != iFlagCols)
            pcAll = "All";
    }
    fprintf(pf, "\n");
    if (0 == iFlagCols)
    {
        fprintf(pf, "        // Первые [0, 2*%d[ колонки\n", Win_cen);
        fprintf(pf, "        for (iCol = 0; iCol < (%d << 1); iCol++)\n", Win_cen);
        fprintf(pf, "        {\n");
        fprintf(pf, "            iCmin = std::max<int64_t>(%d, iCol - %d);\n", Win_cen, Win_cen);
        fprintf(pf, "            iCmax = iCol + %d;\n", Win_cen);
        fprintf(pf, "\n");
    }
    else if (1 == iFlagCols)
    {
        pcAdd_iCmin = " + iCmin";
        pcCurr = "Curr";
        pcAdd_iCminOr0 = "0";
        fprintf(pf, "        // Средние [2*%d, ar_cmmIn.m_i64W - %d] колонки\n", Win_cen, Win_size);
        fprintf(pf, "        puiSumCurr = puiSum + %d;\n", Win_cen);
        if (0 != iFlagRows)
        {
            fprintf(pf, "        pHist = pHistAll + (iCol - %d);\n", Win_cen);
            fprintf(pf, "        for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)\n");
        }
        else
            fprintf(pf, "        for (; iCol <= iColMax; iCol++, puiSumCurr++)\n");
        fprintf(pf, "        {\n");
        if (0 == iFlagRows)
        {
            fprintf(pf, "            iCmin = iCol - %d;\n", Win_cen);
            fprintf(pf, "\n");
        }
    }
    else
    {
        pcAdd_iCminOr0 = "iCmin";
        fprintf(pf, "        // Последние [ar_cmmIn.m_i64W - 2*%d, ar_cmmIn.m_i64W - 1] колонки\n", Win_cen);
        fprintf(pf, "        for (; iCol < ar_cmmIn.m_i64W; iCol++)\n");
        fprintf(pf, "        {\n");
        fprintf(pf, "            iCmin = iCol - %d;\n", Win_cen);
        fprintf(pf, "            iCmax = std::min(ar_cmmIn.m_i64W - %d - 1, iCol + %d);\n", Win_cen, Win_cen);
        fprintf(pf, "\n");
    }
    fprintf(pf, "            if ((d = (pRowIn[iCol] - pseudo_min)*kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!\n");
    if (0 == iFlagRows)
        fprintf(pf, "                pbBrightnessRow[iCol] = 0;\n");
    else
        fprintf(pf, "                u32ValueAdd = 0;\n");
    fprintf(pf, "            else\n");
    fprintf(pf, "            {\n");
    fprintf(pf, "                if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, "                {\n");
        fprintf(pf, "                     pbBrightnessRow[iCol] = u32ValueAdd = 255;\n");
        fprintf(pf, "                     pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W%s);\n", pcAdd_iCmin);
        fprintf(pf, "                }\n");
        fprintf(pf, "                else\n");
        fprintf(pf, "                {\n");
        fprintf(pf, "                     pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);\n");
        fprintf(pf, "                     pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W%s);\n",
                pcAdd_iCmin);
        fprintf(pf, "                }\n");
    }
    else
    {
        fprintf(pf, "                    u32ValueAdd = 255;\n");
        fprintf(pf, "            }\n");
        fprintf(pf, "            if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))\n");
        fprintf(pf, "            {\n");
        fprintf(pf, "                  pHistSub = pHist%s + (u32ValueSub * ar_cmmIn.m_i64W);\n", pcAll);
        fprintf(pf, "                  pHistAdd = pHist%s + (u32ValueAdd * ar_cmmIn.m_i64W);\n", pcAll);
        fprintf(pf, "                  pbBrightnessRow[iCol] = u32ValueAdd;\n");
        if ('\0' != pcCorrSum[0])
            fprintf(pf, "                   uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;\n");
        fprintf(pf, "\n");
    }

    fprintf(pf, "                // Добавляем яркость в %d гистограмм и сумм\n", Win_size);

    if (1 != iFlagCols)
        fprintf(pf, "                for (i = iCmin; i <= iCmax; i++)\n");
    else
        fprintf(pf, " for (i = 0; i < %d; i++)\n", Win_size);

    if ('\0' == pcCorrSum[0])
        fprintf(pf, " puiSum%s[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));\n", pcCurr);
    else
        fprintf(pf, " puiSum%s[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) - u32ValueSub * static_cast<uint32_t>(pHistSub[i]--)) << 1);\n", pcCurr);
    fprintf(pf, " }\n");
    if (0 != iFlagCols)
    {
        if (0 == iFlagRows)
        {
            fprintf(pf, " if ((%d - 1) == iRow)\n", Win_size);
            fprintf(pf, " {\n");
            fprintf(pf, " pfRowOut[0] = static_cast<float>(puiSum%s[%s] * Size_obratn);\n",
                    pcCurr, pcAdd_iCminOr0);
            fprintf(pf, " pfRowOut++;\n");
            fprintf(pf, " }\n");
        }
        else
        {
            fprintf(pf, " pfRowOut[0] = static_cast<float>(puiSum%s[%s] * Size_obratn);\n", pcCurr,
                    pcAdd_iCminOr0);
            fprintf(pf, " pfRowOut++;\n");
        }
    }
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGenRows_V8(FILE *pf, int Win_cen, int iFlagRows)
{
    int Win_size = (Win_cen << 1) + 1;
    int Win_lin = Win_size * Win_size;

    fprintf(pf, "\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, " // Первые [0, %d - 1] строки\n", Win_size);
        fprintf(pf, " pbBrightnessRow = pbBrightness;\n");
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(%d) + %d;\n", Win_cen, Win_cen);
        fprintf(pf, " for (iRow = 0; iRow < %d; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)\n", Win_size);
    }
    else
    {
        fprintf(pf, " // Последующие строки [%d, ar_cmmIn.m_i64H[\n", Win_size);
        fprintf(pf, " for (iRow = %d; iRow < ar_cmmIn.m_i64H; iRow++)\n", Win_size);
    }
    fprintf(pf, " {\n");
    fprintf(pf, " // Обнуляем края строк.\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * %d);\n", Win_cen);
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - %d), 0, sizeof(float) * %d);\n", Win_cen, Win_cen);
    fprintf(pf, "\n");
    fprintf(pf, " uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);\n");
    if (0 != iFlagRows)
    {
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(iRow - %d) + %d;\n", Win_cen, Win_cen);
        fprintf(pf, " iPos = (iRow - %d) %% %d;\n", Win_size, Win_size);
        fprintf(pf, " pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);\n");
    }
    // Первые колонки
    if (0 != iCodeGenRowsCols_V8(pf, Win_cen, iFlagRows, 0))
        return 10;
    // Средние колонки
    if (0 != iCodeGenRowsCols_V8(pf, Win_cen, iFlagRows, 1))
        return 20;
    // Последние колонки
    if (0 != iCodeGenRowsCols_V8(pf, Win_cen, iFlagRows, 2))
        return 30;
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGen_V8(const char *a_pcFilePath)
{
    FILE *pf = fopen(a_pcFilePath, "wt");
    if (nullptr == pf)
        return 10;
    for (int Win_cen = 1; Win_cen <= 10; Win_cen++)
    {
        fprintf(pf, "\n");
        int Win_size = (Win_cen << 1) + 1;
        int Win_lin = Win_size * Win_size;

        // Пролог
        fprintf(pf, "void TextureFiltr_Mean_V8_%d(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)\n", Win_size);
        fprintf(pf, "{\n");
        fprintf(pf, " double Size_obratn = 1.0 / %d;\n", Win_lin);
        fprintf(pf, "\n");
        fprintf(pf, " // Кэш для гистограмм\n");
        fprintf(pf, " uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]\n");
        fprintf(pf, " memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);\n");
        fprintf(pf, " uint16_t *pHist, *pHistAdd, *pHistSub;\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Кэш для сумм\n");
        fprintf(pf, " uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;\n");
        fprintf(pf, " memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Кэш для яркостей\n");
        fprintf(pf, " uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * %d];\n", Win_size);
        fprintf(pf, " uint8_t *pbBrightnessRow;\n");
        fprintf(pf, "\n");
        fprintf(pf, " int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - %d;\n", Win_size);
        fprintf(pf, " ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Обнуляем края. Первые строки.\n");
        fprintf(pf, " for (iRow = 0; iRow < %d ; iRow++)\n", Win_cen);
        fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
        fprintf(pf, "\n");
        fprintf(pf, " int64_t i, iCmin, iCmax, iPos = 0;\n");
        fprintf(pf, " uint32_t u32ValueAdd, u32ValueSub;\n");
        fprintf(pf, " double d;\n");
        // Начальные строки
        if (0 != iCodeGenRows_V8(pf, Win_cen, 0))
        {
            fclose(pf);
            return 20;
        }
        // Последующие строки
        if (0 != iCodeGenRows_V8(pf, Win_cen, 1))
        {
            fclose(pf);
            return 30;
        }
        // Эпилог
        fprintf(pf, "\n");
        fprintf(pf, " // Обнуляем края. Последние строки.\n");
        fprintf(pf, " for (iRow = ar_cmmIn.m_i64H - %d; iRow < ar_cmmOut.m_i64H; iRow++)\n", Win_cen);
        fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
        fprintf(pf, "\n");
        fprintf(pf, " if (nullptr != pHistAll)\n");
        fprintf(pf, " delete[] pHistAll;\n");
        fprintf(pf, " if (nullptr != puiSum)\n");
        fprintf(pf, " delete[] puiSum;\n");
        fprintf(pf, " if (nullptr != pbBrightness)\n");
        fprintf(pf, " delete[] pbBrightness;\n");
        fprintf(pf, "}\n");
    }

    fprintf(pf, "\n");
    fprintf(pf, "typedef void(*tdTextureFiltr_Mean_V8)(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct);\n");
    fprintf(pf, "tdTextureFiltr_Mean_V8 g_afunTextureFiltr_Mean_V8[10] = \n");
    fprintf(pf, "{\n");
    fprintf(pf, " TextureFiltr_Mean_V8_3, // 0\n");
    fprintf(pf, " TextureFiltr_Mean_V8_5, // 1\n");
    fprintf(pf, " TextureFiltr_Mean_V8_7, // 2\n");
    fprintf(pf, " TextureFiltr_Mean_V8_9, // 3\n");
    fprintf(pf, " TextureFiltr_Mean_V8_11, // 4\n");
    fprintf(pf, " TextureFiltr_Mean_V8_13, // 5\n");
    fprintf(pf, " TextureFiltr_Mean_V8_15, // 6\n");
    fprintf(pf, " TextureFiltr_Mean_V8_17, // 7\n");
    fprintf(pf, " TextureFiltr_Mean_V8_19, // 8\n");
    fprintf(pf, " TextureFiltr_Mean_V8_21 // 9\n");
    fprintf(pf, "};\n");
    fprintf(pf, "\n");

    fprintf(pf, "void TextureFiltr_Mean_V8_3_21(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)\n");
    fprintf(pf, "{\n");
    fprintf(pf, "    g_afunTextureFiltr_Mean_V8[Win_cen - 1](ar_cmmIn, ar_cmmOut, pseudo_min, kfct);\n");
    fprintf(pf, "};\n");

    fclose(pf);
    printf("FINISHED\n");
    return 0;
}

//================================================================
//================================================================

void FunV8_8(FILE *pf, int iElOffset, int iLast)
{
    const char *pcDeclare = "__m128i ";
    if (0 != iElOffset)
        pcDeclare = "";
    if (0 == iElOffset)
    {
        fprintf(pf, " %sxmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub)); // Загружаем 128 бит (используем %d бит - %d эл. массива)\n", pcDeclare, 16 * iLast, iLast);
        fprintf(pf, " %sxmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd));\n",
                pcDeclare);
    }
    else
    {
        fprintf(pf, " %sxmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub + %d)); // Загружаем 128 бит (используем %d бит - %d эл. массива)\n", pcDeclare, iElOffset, 16 * iLast,
                iLast);
        fprintf(pf, " %sxmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd + %d));\n", pcDeclare, iElOffset);
    }
    fprintf(pf, "\n");
    if (iLast > 4)
    {
        fprintf(pf, " // Младшие 4 элемента гистограммы\n");
        if (0 == iElOffset)
            fprintf(pf, " %sxmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr)); // Загружаем 128 бит (используем 128 бит - 4 эл. массива)\n", pcDeclare);
        else
            fprintf(pf, " %sxmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + %d)); // Загружаем 128 бит (используем 128 бит - 4 эл. массива)\n", pcDeclare, iElOffset);
    }
    else
    {
        if (1 == iLast)
            fprintf(pf, " // 1 элемент гистограммы\n");
        else
            fprintf(pf, " // %d элемента гистограммы\n", iLast);
        if (0 == iElOffset)
            fprintf(pf, " %sxmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr)); // Загружаем 128 бит (используем %d бит - %d эл. массива)\n", pcDeclare, 32 * iLast,
                    iLast);
        else
            fprintf(pf, " %sxmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + %d)); // Загружаем 128 бит (используем %d бит - %d эл. массива)\n", pcDeclare, iElOffset, 32 * iLast,
                    iLast);
    }
    fprintf(pf, "\n");
    fprintf(pf, " %sxmmHistSub32 = _mm_cvtepu16_epi32(xmmHistSub); // HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)\n", pcDeclare);
    fprintf(pf, " %sxmmHistAdd32 = _mm_cvtepu16_epi32(xmmHistAdd);\n", pcDeclare);
    fprintf(pf, "\n");

    fprintf(pf, "\n");
    if (0 == iElOffset)
    {
        fprintf(pf, " %sxmmValueAdd = _mm_loadu_si32(&u32ValueAdd); // 0, 0, 0, u32ValueAdd (Hi <-> Lo)\n", pcDeclare);
        fprintf(pf, " %sxmmValueSub = _mm_loadu_si32(&u32ValueSub);\n", pcDeclare);
        fprintf(pf, "\n");
        const char *pcImm8 = "0x0";
        if (iLast > 1)
        {
            if (3 == iLast)
                pcImm8 = "0x0C0";
            fprintf(pf, " xmmValueAdd = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueAdd), _mm_castsi128_ps(xmmValueAdd), %s)); // 0, u32ValueAdd, u32ValueAdd, u32ValueAdd\n", pcImm8);
            fprintf(pf, " xmmValueSub = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueSub), _mm_castsi128_ps(xmmValueSub), %s));\n", pcImm8);
            fprintf(pf, "\n");
        }
        if (iLast > 4)
        {
            fprintf(pf, " %sxmmValueAddSub = _mm_add_epi32(xmmValueAdd, xmmValueSub);\n",
                    pcDeclare);
            fprintf(pf, "\n");
        }
    }
    else
    {
        if (1 == iLast)
        {
            fprintf(pf, " xmmValueAdd = _mm_bsrli_si128(xmmValueAdd, 12); // 0, 0, 0, xmmValueAdd (Hi <-> Lo)\n");
            fprintf(pf, " xmmValueSub = _mm_bsrli_si128(xmmValueSub, 12); // 0, 0, 0, xmmValueSub (Hi <-> Lo)\n");
            fprintf(pf, " xmmValueAddSub = _mm_bsrli_si128(xmmValueAddSub, 12);\n");
            fprintf(pf, "\n");
        }
        else if (3 == iLast)
        {
            fprintf(pf, " xmmValueAdd = _mm_bsrli_si128(xmmValueAdd, 4); // 0, xmmValueAdd, xmmValueAdd, xmmValueAdd (Hi <-> Lo)\n");
            fprintf(pf, " xmmValueSub = _mm_bsrli_si128(xmmValueSub, 4); // 0, xmmValueSub, xmmValueSub, xmmValueSub (Hi <-> Lo)\n");
            fprintf(pf, " xmmValueAddSub = _mm_bsrli_si128(xmmValueAddSub, 4);\n");
            fprintf(pf, "\n");
        }
    }
    fprintf(pf, " xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));\n");
    fprintf(pf, " xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1\n");
    if ((0 == iElOffset) && (iLast < 4))
    {
        fprintf(pf, " xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAdd);\n");
        fprintf(pf, " xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueSub);\n");
    }
    else
        fprintf(pf, " xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);\n");
    fprintf(pf, "\n");
    if (0 == iElOffset)
        fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]\n");
    else
        fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + %d), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]\n", iElOffset);
    const char *pcHist = "xmmHistOne_All";
    if (1 == iLast)
        pcHist = "xmmHistOne_1";
    else if (3 == iLast)
        pcHist = "xmmHistOne_3";
    else if (5 == iLast)
        pcHist = "xmmHistOne_5";
    else if (7 == iLast)
        pcHist = "xmmHistOne_7";
    if (iLast > 4)
    {
        fprintf(pf, "\n");
        if (iLast - 4 == 1)
            fprintf(pf, " // Старший 1 элемент гистограммы\n");
        else
            fprintf(pf, " // Старшие %d элемента гистограммы\n", iLast - 4);
        fprintf(pf, " xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + %d)); // Загружаем 128 бит (используем %d - %d эл. массива)\n", iElOffset + 4, 32 * (iLast - 4), iLast - 4);
        fprintf(pf, "\n");
        fprintf(pf, " xmmHistSub32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistSub, 8)); // HistSub32[7], HistSub32[6], HistSub32[5], HistSub32[4] (Hi <-> Lo)\n");
        fprintf(pf, " xmmHistAdd32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistAdd, 8));\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Корректируем значение гистограммы\n");
        if (0 == iElOffset)
        {
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd), _mm_add_epi16(xmmHistAdd, %s)); // pHistAdd[0-%d]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    pcHist, iLast - 1);
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub), _mm_sub_epi16(xmmHistSub, %s)); // pHistSub[0-%d]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    pcHist, iLast - 1);
        }
        else
        {
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd + %d), _mm_add_epi16(xmmHistAdd, %s)); // pHistAdd[0-%d]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    iElOffset, pcHist, iLast - 1);
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub + %d), _mm_sub_epi16(xmmHistSub, %s)); // pHistSub[0-%d]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    iElOffset, pcHist, iLast - 1);
        }
        fprintf(pf, "\n");
        if (iLast < 8)
        {
            if (5 == iLast)
            {
                fprintf(pf, " xmmValueAdd = _mm_bsrli_si128(xmmValueAdd, 12); // 0, 0, 0, xmmValueAdd (Hi <-> Lo)\n");
                fprintf(pf, " xmmValueSub = _mm_bsrli_si128(xmmValueSub, 12); // 0, 0, 0, xmmValueSub (Hi <-> Lo)\n");
                fprintf(pf, " xmmValueAddSub = _mm_bsrli_si128(xmmValueAddSub, 12);\n");
                fprintf(pf, "\n");
            }
            else if (7 == iLast)
            {
                fprintf(pf, " xmmValueAdd = _mm_bsrli_si128(xmmValueAdd, 4); // 0, xmmValueAdd, xmmValueAdd, xmmValueAdd (Hi <-> Lo)\n");
                fprintf(pf, " xmmValueSub = _mm_bsrli_si128(xmmValueSub, 4); // 0, xmmValueSub, xmmValueSub, xmmValueSub (Hi <-> Lo)\n");
                fprintf(pf, " xmmValueAddSub = _mm_bsrli_si128(xmmValueAddSub, 4);\n");
                fprintf(pf, "\n");
            }
        }
        fprintf(pf, " xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));\n");
        fprintf(pf, " xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1\n");
        fprintf(pf, " xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);\n");
        fprintf(pf, "\n");
        fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + %d), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]\n", iElOffset + 4);
    }
    else
    {
        if (0 == iElOffset)
        {
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd), _mm_add_epi16(xmmHistAdd, %s)); // pHistAdd[0-%d]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    pcHist, iLast - 1);
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub), _mm_sub_epi16(xmmHistSub, %s)); // pHistSub[0-%d]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    pcHist, iLast - 1);
        }
        else
        {
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd + %d), _mm_add_epi16(xmmHistAdd, %s)); // pHistAdd[0-%d]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n", iElOffset, pcHist, iLast - 1);
            fprintf(pf, " _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub + %d), _mm_sub_epi16(xmmHistSub, %s)); // pHistSub[0-%d]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)\n",
                    iElOffset, pcHist, iLast - 1);
        }
    }
}

int iCodeGenRowsCols_V8_sse4(FILE *pf, int Win_cen, int iFlagRows, int iFlagCols)
{
    int Win_size = (Win_cen << 1) + 1;
    int Win_lin = Win_size * Win_size;

    const char *pcAdd_iCmin = "";
    const char *pcAdd_iCminOr0 = "";
    const char *pcCurr = "";
    const char *pcCorrSum = "";
    const char *pcAll = "";
    if (0 != iFlagRows)
    {
        pcCorrSum = " - u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1)";
        if (1 != iFlagCols)
            pcAll = "All";
    }
    fprintf(pf, "\n");
    if (0 == iFlagCols)
    {
        fprintf(pf, "        // Первые [0, 2*%d[ колонки\n", Win_cen);
        fprintf(pf, "        for (iCol = 0; iCol < (%d << 1); iCol++)\n", Win_cen);
        fprintf(pf, "        {\n");
        fprintf(pf, "            iCmin = std::max<int64_t>(%d, iCol - %d);\n", Win_cen, Win_cen);
        fprintf(pf, "            iCmax = iCol + %d;\n", Win_cen);
        fprintf(pf, "\n");
    }
    else if (1 == iFlagCols)
    {
        pcAdd_iCmin = " + iCmin";
        pcCurr = "Curr";
        pcAdd_iCminOr0 = "0";
        fprintf(pf, "        // Средние [2*%d, ar_cmmIn.m_i64W - %d] колонки\n", Win_cen, Win_size);
        fprintf(pf, "        puiSumCurr = puiSum + %d;\n", Win_cen);
        if (0 != iFlagRows)
        {
            fprintf(pf, "        pHist = pHistAll + (iCol - %d);\n", Win_cen);
            fprintf(pf, "        for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)\n");
        }
        else
            fprintf(pf, "        for (; iCol <= iColMax; iCol++, puiSumCurr++)\n");
        fprintf(pf, "        {\n");
        if (0 == iFlagRows)
        {
            fprintf(pf, "            iCmin = iCol - %d;\n", Win_cen);
            fprintf(pf, "\n");
        }
    }
    else
    {
        pcAdd_iCminOr0 = "iCmin";
        fprintf(pf, "        // Последние [ar_cmmIn.m_i64W - 2*%d, ar_cmmIn.m_i64W - 1] колонки\n", Win_cen);
        fprintf(pf, "        for (; iCol < ar_cmmIn.m_i64W; iCol++)\n");
        fprintf(pf, "        {\n");
        fprintf(pf, "            iCmin = iCol - %d;\n", Win_cen);
        fprintf(pf, "            iCmax = std::min(ar_cmmIn.m_i64W - %d - 1, iCol + %d);\n", Win_cen, Win_cen);
        fprintf(pf, "\n");
    }
    fprintf(pf, "            if ((d = (pRowIn[iCol] - pseudo_min)*kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!\n");
    if (0 == iFlagRows)
        fprintf(pf, "                pbBrightnessRow[iCol] = 0;\n");
    else
        fprintf(pf, "                u32ValueAdd = 0;\n");
    fprintf(pf, "            else\n");
    fprintf(pf, "            {\n");
    fprintf(pf, "                if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, "                {\n");
        fprintf(pf, "                     pbBrightnessRow[iCol] = u32ValueAdd = 255;\n");
        fprintf(pf, "                     pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W%s);\n", pcAdd_iCmin);
        fprintf(pf, "                }\n");
        fprintf(pf, "                else\n");
        fprintf(pf, "                {\n");
        fprintf(pf, "                     pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);\n");
        fprintf(pf, "                     pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W%s);\n",
                pcAdd_iCmin);
        fprintf(pf, "                }\n");
    }
    else
    {
        fprintf(pf, "                    u32ValueAdd = 255;\n");
        fprintf(pf, "            }\n");
        fprintf(pf, "            if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))\n");
        fprintf(pf, "            {\n");
        fprintf(pf, "                  pHistSub = pHist%s + (u32ValueSub * ar_cmmIn.m_i64W);\n", pcAll);
        fprintf(pf, "                  pHistAdd = pHist%s + (u32ValueAdd * ar_cmmIn.m_i64W);\n", pcAll);
        fprintf(pf, "                  pbBrightnessRow[iCol] = u32ValueAdd;\n");
        if ('\0' != pcCorrSum[0] && ((1 != iFlagRows) || (1 != iFlagCols)))
            fprintf(pf, "                   uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;\n");
        fprintf(pf, "\n");
    }

    fprintf(pf, "                // Добавляем яркость в %d гистограмм и сумм\n", Win_size);

    if (1 != iFlagCols)
        fprintf(pf, "                for (i = iCmin; i <= iCmax; i++)\n");
    else
    { // 1 == iFlagCols
        if (1 != iFlagRows)
            fprintf(pf, " for (i = 0; i < %d; i++)\n", Win_size);
        else
        { // SSE*
            int iOffset = 0;
            int iLast = Win_size;
            if (Win_size > 8)
            {
                fprintf(pf, " //============================\n");
                fprintf(pf, " // Первые 8 точек\n");
                fprintf(pf, " //============================\n");
                FunV8_8(pf, 0, 8);
                iOffset = 8;
                iLast -= 8;
                if (Win_size > 16)
                {
                    fprintf(pf, " //============================\n");
                    fprintf(pf, " // Вторые 8 точек\n");
                    fprintf(pf, " //============================\n");
                    FunV8_8(pf, 8, 8);
                    iOffset = 16;
                    iLast -= 8;
                }
                fprintf(pf, " //============================\n");
                if (1 == iLast)
                    fprintf(pf, " // Последняя 1 точка\n");
                else if (3 == iLast)
                    fprintf(pf, " // Последние 3 точки\n");
                else
                    fprintf(pf, " // Последние %d точек\n", iLast);
                fprintf(pf, " //============================\n");
            }
            else
            {
                fprintf(pf, " //============================\n");
                if (3 == iLast)
                    fprintf(pf, " // Первые 3 точки\n");
                else
                    fprintf(pf, " // Первые %d точек\n", iLast);
                fprintf(pf, " //============================\n");
            }
            FunV8_8(pf, iOffset, iLast);
        }
    }
    if ((1 != iFlagRows) || (1 != iFlagCols))
    {
        if ('\0' == pcCorrSum[0])
            fprintf(pf, " puiSum%s[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));\n", pcCurr);
        else
            fprintf(pf, " puiSum%s[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) - u32ValueSub * static_cast<uint32_t>(pHistSub[i]--)) << 1);\n", pcCurr);
    }
    fprintf(pf, " }\n");
    if (0 != iFlagCols)
    {
        if (0 == iFlagRows)
        {
            fprintf(pf, " if ((%d - 1) == iRow)\n", Win_size);
            fprintf(pf, " {\n");
            fprintf(pf, " pfRowOut[0] = static_cast<float>(puiSum%s[%s] * Size_obratn);\n",
                    pcCurr, pcAdd_iCminOr0);
            fprintf(pf, " pfRowOut++;\n");
            fprintf(pf, " }\n");
        }
        else
        {
            fprintf(pf, " pfRowOut[0] = static_cast<float>(puiSum%s[%s] * Size_obratn);\n", pcCurr,
                    pcAdd_iCminOr0);
            fprintf(pf, " pfRowOut++;\n");
        }
    }
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGenRows_V8_sse4(FILE *pf, int Win_cen, int iFlagRows)
{
    int Win_size = (Win_cen << 1) + 1;
    int Win_lin = Win_size * Win_size;

    fprintf(pf, "\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, " // Первые [0, %d - 1] строки\n", Win_size);
        fprintf(pf, " pbBrightnessRow = pbBrightness;\n");
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(%d) + %d;\n", Win_cen, Win_cen);
        fprintf(pf, " for (iRow = 0; iRow < %d; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)\n", Win_size);
    }
    else
    {
        int i, j8 = Win_size % 8;
        fprintf(pf, " __m128i xmmHistOne_%d = _mm_set_epi16(", j8);
        for (i = 0; i < 8 - j8; i++)
        {
            fprintf(pf, "0");
            if (i < 7)
                fprintf(pf, ", ");
        }
        for (; i < 8; i++)
        {
            fprintf(pf, "1");
            if (i < 7)
                fprintf(pf, ", ");
        }
        fprintf(pf, "); // Для inc или dec сразу для %d гистограмм\n", j8);
        if (Win_size > 8)
            fprintf(pf, " __m128i xmmHistOne_All = _mm_set_epi16(1, 1, 1, 1, 1, 1, 1, 1); // Для inc или dec сразу для всех гистограмм\n");
        fprintf(pf, "\n");

        fprintf(pf, " // Последующие строки [%d, ar_cmmIn.m_i64H[\n", Win_size);
        fprintf(pf, " for (iRow = %d; iRow < ar_cmmIn.m_i64H; iRow++)\n", Win_size);
    }
    fprintf(pf, " {\n");
    fprintf(pf, " // Обнуляем края строк.\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * %d);\n", Win_cen);
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - %d), 0, sizeof(float) * %d);\n", Win_cen, Win_cen);
    fprintf(pf, "\n");
    fprintf(pf, " uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);\n");
    if (0 != iFlagRows)
    {
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(iRow - %d) + %d;\n", Win_cen, Win_cen);
        fprintf(pf, " iPos = (iRow - %d) %% %d;\n", Win_size, Win_size);
        fprintf(pf, " pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);\n");
    }
    // Первые колонки
    if (0 != iCodeGenRowsCols_V8_sse4(pf, Win_cen, iFlagRows, 0))
        return 10;
    // Средние колонки
    if (0 != iCodeGenRowsCols_V8_sse4(pf, Win_cen, iFlagRows, 1))
        return 20;
    // Последние колонки
    if (0 != iCodeGenRowsCols_V8_sse4(pf, Win_cen, iFlagRows, 2))
        return 30;
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGen_V8_sse4(const char *a_pcFilePath)
{
    FILE *pf = fopen(a_pcFilePath, "wt");
    if (nullptr == pf)
        return 10;
    for (int Win_cen = 1; Win_cen <= 10; Win_cen++)
    {
        fprintf(pf, "\n");
        int Win_size = (Win_cen << 1) + 1;
        int Win_lin = Win_size * Win_size;

        // Пролог
        fprintf(pf, "void TextureFiltr_Mean_V8_sse4_%d(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)\n", Win_size);
        fprintf(pf, "{\n");
        fprintf(pf, " double Size_obratn = 1.0 / %d;\n", Win_lin);
        fprintf(pf, "\n");
        fprintf(pf, " // Кэш для гистограмм\n");
        fprintf(pf, " uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]\n");
        fprintf(pf, " memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);\n");
        fprintf(pf, " uint16_t *pHist, *pHistAdd, *pHistSub;\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Кэш для сумм\n");
        fprintf(pf, " uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;\n");
        fprintf(pf, " memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Кэш для яркостей\n");
        fprintf(pf, " uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * %d];\n", Win_size);
        fprintf(pf, " uint8_t *pbBrightnessRow;\n");
        fprintf(pf, "\n");
        fprintf(pf, " int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - %d;\n", Win_size);
        fprintf(pf, " ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);\n");
        fprintf(pf, "\n");
        fprintf(pf, " // Обнуляем края. Первые строки.\n");
        fprintf(pf, " for (iRow = 0; iRow < %d ; iRow++)\n", Win_cen);
        fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
        fprintf(pf, "\n");
        fprintf(pf, " int64_t i, iCmin, iCmax, iPos = 0;\n");
        fprintf(pf, " uint32_t u32ValueAdd, u32ValueSub;\n");
        fprintf(pf, " double d;\n");
        // Начальные строки
        if (0 != iCodeGenRows_V8_sse4(pf, Win_cen, 0))
        {
            fclose(pf);
            return 20;
        }
        // Последующие строки
        if (0 != iCodeGenRows_V8_sse4(pf, Win_cen, 1))
        {
            fclose(pf);
            return 30;
        }
        // Эпилог
        fprintf(pf, "\n");
        fprintf(pf, " // Обнуляем края. Последние строки.\n");
        fprintf(pf, " for (iRow = ar_cmmIn.m_i64H - %d; iRow < ar_cmmOut.m_i64H; iRow++)\n", Win_cen);
        fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
        fprintf(pf, "\n");
        fprintf(pf, " if (nullptr != pHistAll)\n");
        fprintf(pf, " delete[] pHistAll;\n");
        fprintf(pf, " if (nullptr != puiSum)\n");
        fprintf(pf, " delete[] puiSum;\n");
        fprintf(pf, " if (nullptr != pbBrightness)\n");
        fprintf(pf, " delete[] pbBrightness;\n");
        fprintf(pf, "}\n");
    }

    fprintf(pf, "\n");
    fprintf(pf, "typedef void(*tdTextureFiltr_Mean_V8_sse4)(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct);\n");
    fprintf(pf, "tdTextureFiltr_Mean_V8_sse4 g_afunTextureFiltr_Mean_V8_sse4[10] = \n");
    fprintf(pf, "{\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_3, // 0\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_5, // 1\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_7, // 2\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_9, // 3\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_11, // 4\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_13, // 5\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_15, // 6\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_17, // 7\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_19, // 8\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_21 // 9\n");
    fprintf(pf, "};\n");
    fprintf(pf, "\n");

    fprintf(pf, "void TextureFiltr_Mean_V8_sse4_3_21(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)\n");
    fprintf(pf, "{\n");
    fprintf(pf, "    g_afunTextureFiltr_Mean_V8_sse4[Win_cen - 1](ar_cmmIn, ar_cmmOut, pseudo_min, kfct);\n");
    fprintf(pf, "};\n");

    fclose(pf);
    printf("FINISHED\n");
    return 0;
}

//================================================================
//================================================================

int iCodeGenRows_V8_sse4_Omp(FILE *pf, int Win_cen, int iFlagRows)
{
    int Win_size = (Win_cen << 1) + 1;
    int Win_lin = Win_size * Win_size;

    fprintf(pf, "\n");
    if (0 == iFlagRows)
    {
        fprintf(pf, "        // Первые [0, %d - 1] строки\n", Win_size);
        fprintf(pf, "        pbBrightnessRow = pbBrightness;\n");
        fprintf(pf, "        float *pfRowOut = ar_cmmOut.pfGetRow(g_StartRow + %d) + %d;\n", Win_cen, Win_cen);
        fprintf(pf, "        for (iRow = 0, g_iRow = g_StartRow; iRow < %d; iRow++, g_iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)\n", Win_size);
    }
    else
    {
        int i, j8 = Win_size % 8;
        fprintf(pf, " __m128i xmmHistOne_%d = _mm_set_epi16(", j8);
        for (i = 0; i < 8 - j8; i++)
        {
            fprintf(pf, "0");
            if (i < 7)
                fprintf(pf, ", ");
        }
        for (; i < 8; i++)
        {
            fprintf(pf, "1");
            if (i < 7)
                fprintf(pf, ", ");
        }
        fprintf(pf, "); // Для inc или dec сразу для %d гистограмм\n", j8);
        if (Win_size > 8)
            fprintf(pf, " __m128i xmmHistOne_All = _mm_set_epi16(1, 1, 1, 1, 1, 1, 1, 1); // Для inc или dec сразу для всех гистограмм\n");
        fprintf(pf, "\n");

        fprintf(pf, " // Последующие строки [%d, ar_cmmIn.m_i64H[\n", Win_size);
        fprintf(pf, " for (iRow = %d; g_iRow < g_EndRow; iRow++, g_iRow++)\n", Win_size);
    }
    fprintf(pf, " {\n");
    fprintf(pf, " // Обнуляем края строк.\n");
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(g_iRow), 0, sizeof(float) * %d);\n", Win_cen);
    fprintf(pf, " memset(ar_cmmOut.pfGetRow(g_iRow) + (ar_cmmIn.m_i64W - %d), 0, sizeof(float) * %d);\n", Win_cen, Win_cen);
    fprintf(pf, "\n");
    fprintf(pf, " uint16_t *pRowIn = ar_cmmIn.pu16GetRow(g_iRow);\n");
    if (0 != iFlagRows)
    {
        fprintf(pf, " float *pfRowOut = ar_cmmOut.pfGetRow(g_iRow - %d) + %d;\n", Win_cen, Win_cen);
        fprintf(pf, " iPos = (iRow - %d) %% %d;\n", Win_size, Win_size);
        fprintf(pf, " pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);\n");
    }
    // Первые колонки
    if (0 != iCodeGenRowsCols_V8_sse4(pf, Win_cen, iFlagRows, 0))
        return 10;
    // Средние колонки
    if (0 != iCodeGenRowsCols_V8_sse4(pf, Win_cen, iFlagRows, 1))
        return 20;
    // Последние колонки
    if (0 != iCodeGenRowsCols_V8_sse4(pf, Win_cen, iFlagRows, 2))
        return 30;
    fprintf(pf, " }\n");
    return 0;
}

int iCodeGen_V8_sse4_Omp(const char *a_pcFilePath)
{
    FILE *pf = fopen(a_pcFilePath, "wt");
    if (nullptr == pf)
        return 10;
    for (int Win_cen = 1; Win_cen <= 10; Win_cen++)
    {
        fprintf(pf, "\n");
        int Win_size = (Win_cen << 1) + 1;
        int Win_lin = Win_size * Win_size;

        // Пролог
        fprintf(pf, "void TextureFiltr_Mean_V8_sse4_Omp_%d(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct, int a_iThreads)\n", Win_size);
        fprintf(pf, "{\n");
        fprintf(pf, "    int64_t n_withoutBorderRows = ar_cmmIn.m_i64H - %d * 2;\n", Win_cen);
        fprintf(pf, "    int64_t fairRowsPerWorker = n_withoutBorderRows / a_iThreads;\n");
        fprintf(pf, "    double Size_obratn = 1.0 / %d;\n", Win_lin);
        fprintf(pf, "\n");
        fprintf(pf, "    ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);\n");
        fprintf(pf, "    // Обнуляем края. Первые строки.\n");
        fprintf(pf, "    for (int64_t iRow = 0; iRow < %d ; iRow++)\n", Win_cen);
        fprintf(pf, "        memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
        fprintf(pf, "\n");
        fprintf(pf, "#pragma omp parallel for num_threads(a_iThreads)\n");
        fprintf(pf, "    for (int iThread = 0; iThread < a_iThreads; iThread++)\n");
        fprintf(pf, "    {\n");
        fprintf(pf, "        int64_t g_StartRow = fairRowsPerWorker * iThread;\n");
		fprintf(pf, "        int64_t g_EndRow = iThread + 1 == a_iThreads ? ar_cmmIn.m_i64H : fairRowsPerWorker * (iThread + 1) + %d * 2;\n", Win_cen);
        fprintf(pf, "\n");
        fprintf(pf, "        // Кэш для гистограмм\n");
        fprintf(pf, "        uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]\n");
        fprintf(pf, "        memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);\n");
        fprintf(pf, "        uint16_t *pHist, *pHistAdd, *pHistSub;\n");
        fprintf(pf, "\n");
        fprintf(pf, "        // Кэш для сумм\n");
        fprintf(pf, "        uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;\n");
        fprintf(pf, "        memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);\n");
        fprintf(pf, "\n");
        fprintf(pf, "        // Кэш для яркостей\n");
        fprintf(pf, "        uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * %d];\n", Win_size);
        fprintf(pf, "        uint8_t *pbBrightnessRow;\n");
        fprintf(pf, "\n");
        fprintf(pf, "        int64_t iRow, g_iRow, iCol, iColMax = ar_cmmIn.m_i64W - %d;\n", Win_size);
        fprintf(pf, "\n");
        fprintf(pf, "        int64_t i, iCmin, iCmax, iPos = 0;\n");
        fprintf(pf, "        uint32_t u32ValueAdd, u32ValueSub;\n");
        fprintf(pf, "        double d;\n");
        // Начальные строки
        if (0 != iCodeGenRows_V8_sse4_Omp(pf, Win_cen, 0))
        {
            fclose(pf);
            return 20;
        }
        // Последующие строки
        if (0 != iCodeGenRows_V8_sse4_Omp(pf, Win_cen, 1))
        {
            fclose(pf);
            return 30;
        }
        // Эпилог
        fprintf(pf, "\n");
        fprintf(pf, " if (nullptr != pHistAll)\n");
        fprintf(pf, " delete[] pHistAll;\n");
        fprintf(pf, " if (nullptr != puiSum)\n");
        fprintf(pf, " delete[] puiSum;\n");
        fprintf(pf, " if (nullptr != pbBrightness)\n");
        fprintf(pf, " delete[] pbBrightness;\n");
        fprintf(pf, "}\n");
        fprintf(pf, " // Обнуляем края. Последние строки.\n");
        fprintf(pf, " for (int64_t iRow = ar_cmmIn.m_i64H - %d; iRow < ar_cmmOut.m_i64H; iRow++)\n", Win_cen);
        fprintf(pf, " memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);\n");
        fprintf(pf, "}\n");
    }

    fprintf(pf, "\n");
    fprintf(pf, "typedef void(*tdTextureFiltr_Mean_V8_sse4_Omp)(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct, int a_iThreads);\n");
    fprintf(pf, "tdTextureFiltr_Mean_V8_sse4_Omp g_afunTextureFiltr_Mean_V8_sse4_Omp[10] = \n");
    fprintf(pf, "{\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_3, // 0\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_5, // 1\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_7, // 2\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_9, // 3\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_11, // 4\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_13, // 5\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_15, // 6\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_17, // 7\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_19, // 8\n");
    fprintf(pf, " TextureFiltr_Mean_V8_sse4_Omp_21 // 9\n");
    fprintf(pf, "};\n");
    fprintf(pf, "\n");

    fprintf(pf, "void TextureFiltr_Mean_V8_sse4_Omp_3_21(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct, int a_iThreads)\n");
    fprintf(pf, "{\n");
    fprintf(pf, "    g_afunTextureFiltr_Mean_V8_sse4_Omp[Win_cen - 1](ar_cmmIn, ar_cmmOut, pseudo_min, kfct, a_iThreads);\n");
    fprintf(pf, "};\n");

    fclose(pf);
    printf("FINISHED\n");
    return 0;
}

//================================================================
//================================================================


int main()
{
    // return iCodeGen_V7("../generated/TextureFiltr_Mean_V7.cpp");
    // return iCodeGen_V8("../generated/TextureFiltr_Mean_V8_3_21.cpp");
    // return iCodeGen_V8_sse4("../generated/TextureFiltr_Mean_V8_sse4_3_21.cpp");
    return iCodeGen_V8_sse4_Omp("../generated/TextureFiltr_Mean_V8_sse4_Omp_3_21.cpp");
}

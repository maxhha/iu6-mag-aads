// ConsoleApplication1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
// #include <conio.h>
// #include <string>
#include <cstring>
#include <omp.h>
#include <math.h>
#include <algorithm>
#include <immintrin.h>

#include "../inc/MU16Data.h"

//================================================================
class BTexture
{
public:
	BTexture();

	void Create(uint8_t *a_pVarRow, int a_Win_cen);
	void CalcHist();
	double Calc_Mean();

	uint8_t *m_pVarRow;
	uint16_t hist[256];
	int Win_cen;
	int Win_size;
	int Win_Lin;
	double Size_obratn;
};
//================================================================

BTexture::BTexture()
{
	m_pVarRow = nullptr;
}

void BTexture::Create(uint8_t *a_pVarRow, int a_Win_cen)
{
	Win_cen = a_Win_cen;
	Win_size = (Win_cen << 1) + 1;
	Win_Lin = Win_size * Win_size;
	Size_obratn = 1.0 / Win_Lin;
	m_pVarRow = a_pVarRow;
}

void BTexture::CalcHist()
{
	memset(hist, 0, sizeof(uint16_t) * 256);

	for (int i = 0; i < Win_Lin; i++)
	{
		hist[m_pVarRow[i]]++;
	}
}

double BTexture::Calc_Mean()
{
	CalcHist();
	double Sum = 0.0;
	for (int i = 0; i < Win_Lin; i++)
	{
		Sum += m_pVarRow[i] * hist[m_pVarRow[i]] * Size_obratn;
	}
	return Sum;
}

//================================================================
//================================================================

int iCompareDataF32(MFData &mSrc, MFData &mOut, float &percentWrong)
{
	int64_t wrong = 0;
#define EPS 1e-6

	for (int i = 0; i < mSrc.m_i64H; i++)
	{
		for (int j = 0; j < mSrc.m_i64W; j++)
		{
			if (fabs(mSrc.pfGetRow(i)[j] - mOut.pfGetRow(i)[j]) >= EPS)
			{
				// printf("row, col: %d, %d: %.4f\n", i, j, mSrc.pfGetRow(i)[j] - mOut.pfGetRow(i)[j]);
				wrong++;
			}
		}
	}

	percentWrong = float(wrong) / mSrc.m_i64H / mSrc.m_i64W;

	return wrong;
}

//================================================================
//================================================================

void TextureFiltr_Mean(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int i = (Win_cen << 1) + 1;
	int Win_lin = i * i;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	BTexture bt;
	bt.Create(pVarRow, Win_cen);

	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);

	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	int ln, rw;
	for (iRow = Win_cen; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);

		for (iCol = Win_cen; iCol < iColMax; iCol++)
		{
			int i_num = 0;
			for (rw = -Win_cen; rw <= Win_cen; rw++)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d <= 0.)
						pVarRow[i_num] = 0;
					else
					{
						int iVal = static_cast<int>(d + 0.5);
						if (iVal > 255)
							pVarRow[i_num] = 255;
						else
							pVarRow[i_num] = static_cast<uint8_t>(iVal);
					}
					i_num++;
				}
			}
			pfRowOut[iCol] = static_cast<float>(bt.Calc_Mean());
		}

		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}

	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	if (nullptr != pVarRow)
		delete[] pVarRow;
}

//================================================================
//================================================================

void TextureFiltr_Mean_Omp(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct, int a_iThreads)
{
	int i = (Win_cen << 1) + 1;
	int Win_lin = i * i;
	if (a_iThreads <= 0)
		a_iThreads = 1;
	uint8_t *pVarRowTh = new uint8_t[Win_lin * a_iThreads];
	BTexture *pbt = new BTexture[a_iThreads];
	for (i = 0; i < a_iThreads; i++)
		pbt[i].Create(pVarRowTh + (i * Win_lin), Win_cen);

	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);

	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	int ln, rw;
#pragma omp parallel private(ln, rw, iCol) num_threads(a_iThreads)
	{
		int iThreadNum = omp_get_thread_num();
		uint8_t *pVarRow = pVarRowTh + (iThreadNum * Win_lin);
		BTexture &bt = pbt[iThreadNum];
#pragma omp for
		for (iRow = Win_cen; iRow < iRowMax; iRow++)
		{
			float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
			// Обнуляем края. Левый край.
			memset(pfRowOut, 0, sizeof(float) * Win_cen);

			for (iCol = Win_cen; iCol < iColMax; iCol++)
			{
				int i_num = 0;
				for (rw = -Win_cen; rw <= Win_cen; rw++)
				{
					uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
					for (ln = -Win_cen; ln <= Win_cen; ln++)
					{
						double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
						if (d <= 0.)
							pVarRow[i_num] = 0;
						else
						{
							int iVal = static_cast<int>(d + 0.5);
							if (iVal > 255)
								pVarRow[i_num] = 255;
							else
								pVarRow[i_num] = static_cast<uint8_t>(iVal);
						}
						i_num++;
					}
				}
				pfRowOut[iCol] = static_cast<float>(bt.Calc_Mean());
			}

			// Обнуляем края. Правый край.
			memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
		}

		// Обнуляем края. Последние строки.
		for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
			memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	}
	if (nullptr != pVarRowTh)
		delete[] pVarRowTh;
	if (nullptr != pbt)
		delete[] pbt;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V2(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int i = (Win_cen << 1) + 1;
	int Win_lin = i * i;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	uint16_t *pHist = new uint16_t[256];
	double Size_obratn = 1.0 / Win_lin;

	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);

	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	int ln, rw;
	for (iRow = Win_cen; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);

		for (iCol = Win_cen; iCol < iColMax; iCol++)
		{
			memset(pHist, 0, sizeof(uint16_t) * 256);
			int i_num = 0;
			for (rw = -Win_cen; rw <= Win_cen; rw++)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d <= 0.)
						pHist[pVarRow[i_num] = 0]++;
					else
					{
						int iVal = static_cast<int>(d + 0.5);
						if (iVal > 255)
							pHist[pVarRow[i_num] = 255]++;
						else
							pHist[pVarRow[i_num] = static_cast<uint8_t>(iVal)]++;
					}
					i_num++;
				}
			}
			uint32_t Sum = 0;
			for (int i = 0; i < Win_lin; i++)
			{
				Sum += pVarRow[i] * static_cast<uint32_t>(pHist[pVarRow[i]]);
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}

		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}

	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	if (nullptr != pVarRow)
		delete[] pVarRow;
	if (nullptr != pHist)
		delete[] pHist;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V2_1(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int i = (Win_cen << 1) + 1;
	int Win_lin = i * i;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	uint16_t *pHist = new uint16_t[256];
	double Size_obratn = 1.0 / Win_lin;

	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);

	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	int ln, rw;
	for (iRow = Win_cen; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);

		for (iCol = Win_cen; iCol < iColMax; iCol++)
		{
			memset(pHist, 0, sizeof(uint16_t) * 256);
			int i_num = 0;
			for (rw = -Win_cen; rw <= Win_cen; rw++)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d <= 0.)
						pHist[pVarRow[i_num] = 0]++;
					else
					{
						int iVal = static_cast<int>(d + 0.5);
						if (iVal > 255)
							pHist[pVarRow[i_num] = 255]++;
						else
							pHist[pVarRow[i_num] = static_cast<uint8_t>(iVal)]++;
					}
					i_num++;
				}
			}
			uint32_t Sum = 0;
			for (uint32_t i = 0; i < 256; i++)
			{
				uint32_t iH = pHist[i];
				Sum += i * iH * iH;
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}

		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}

	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	if (nullptr != pVarRow)
		delete[] pVarRow;
	if (nullptr != pHist)
		delete[] pHist;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V3(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int i = (Win_cen << 1) + 1;
	int Win_lin = i * i;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	uint16_t *pHist = new uint16_t[256];
	double Size_obratn = 1.0 / Win_lin;

	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);

	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	int ln, rw;
	for (iRow = Win_cen; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);

		for (iCol = Win_cen; iCol < iColMax; iCol++)
		{
			memset(pHist, 0, sizeof(uint16_t) * 256);
			int i_num = 0;
			for (rw = -Win_cen; rw <= Win_cen; rw++)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d <= 0.)
						pHist[pVarRow[i_num] = 0]++;
					else
					{
						int iVal = static_cast<int>(d + 0.5);
						if (iVal > 255)
							pHist[pVarRow[i_num] = 255]++;
						else
							pHist[pVarRow[i_num] = static_cast<uint8_t>(iVal)]++;
					}
					i_num++;
				}
			}
			uint32_t Sum = 0;
			for (int i = 0; i < Win_lin; i++)
			{
				uint32_t iValue = static_cast<uint32_t>(pVarRow[i]);
				uint32_t iH = static_cast<uint32_t>(pHist[iValue]);
				Sum += iValue * iH * iH;
				pHist[iValue] = 0;
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}

		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}

	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	if (nullptr != pVarRow)
		delete[] pVarRow;
	if (nullptr != pHist)
		delete[] pHist;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V4(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int Win_size = (Win_cen << 1) + 1;
	int Win_lin = Win_size * Win_size;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	uint16_t *pHist = new uint16_t[256];
	double Size_obratn = 1.0 / Win_lin;
	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int ln, rw;
	for (iRow = Win_cen; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);
		// Первая точка в строке
		int iPos = 0;
		memset(pHist, 0, sizeof(uint16_t) * 256);
		iCol = Win_cen;
		{
			for (rw = -Win_cen; rw <= Win_cen; rw++)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d <= 0.)
						pHist[pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] = 0]++;
					else
					{
						int iVal = static_cast<int>(d + 0.5);
						if (iVal > 255)
							pHist[pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] = 255]++;
						else
							pHist[pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] =
									  static_cast<uint8_t>(iVal)]++;
					}
				}
			}
			uint32_t Sum = 0;
			for (int i = 0; i < Win_lin; i++)
			{
				uint32_t iValue = static_cast<uint32_t>(pVarRow[i]);
				Sum += iValue * static_cast<uint32_t>(pHist[iValue]);
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}
		for (++iCol; iCol < iColMax; iCol++)
		{
			uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow - Win_cen) + iCol + Win_cen;
			for (rw = 0; rw < Win_size; rw++, pRowIn += ar_cmmIn.m_i64LineSizeEl)
			{
				// Вычитаем из гистограммы столбец iCol - Win_cen - 1
				pHist[pVarRow[iPos + rw]]--;
				// Добавим в гистограмму столбец iCol + Win_cen
				double d = (pRowIn[0] - pseudo_min) * kfct;
				if (d <= 0.)
					pHist[pVarRow[iPos + rw] = 0]++;
				else
				{
					int iVal = static_cast<int>(d + 0.5);
					if (iVal > 255)
						pHist[pVarRow[iPos + rw] = 255]++;
					else
					{
						pHist[pVarRow[iPos + rw] = static_cast<uint8_t>(iVal)]++;
					}
				}
			}
			iPos += Win_size;
			if (iPos >= Win_lin)
				iPos = 0;
			uint32_t Sum = 0;
			for (int i = 0; i < Win_lin; i++)
			{
				uint32_t iValue = static_cast<uint32_t>(pVarRow[i]);
				Sum += iValue * static_cast<uint32_t>(pHist[iValue]);
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}
		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}
	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pVarRow)
		delete[] pVarRow;
	if (nullptr != pHist)
		delete[] pHist;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V5(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int Win_size = (Win_cen << 1) + 1;
	int Win_lin = Win_size * Win_size;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	uint16_t *pHist = new uint16_t[256];
	double Size_obratn = 1.0 / Win_lin;
	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int ln, rw;
	for (iRow = Win_cen; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);
		// Первая точка в строке
		int iPos = 0;
		uint32_t Sum = 0;
		memset(pHist, 0, sizeof(uint16_t) * 256);
		iCol = Win_cen;
		{
			for (rw = -Win_cen; rw <= Win_cen; rw++)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d <= 0.)
						pHist[pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] = 0]++;
					else
					{
						int iVal = static_cast<int>(d + 0.5);
						if (iVal > 255)
							pHist[pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] = 255]++;
						else
							pHist[pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] =
									  static_cast<uint8_t>(iVal)]++;
					}
				}
			}

			for (int i = 0; i < Win_lin; i++)
			{
				uint32_t iValue = static_cast<uint32_t>(pVarRow[i]);
				Sum += iValue * static_cast<uint32_t>(pHist[iValue]);
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}
		for (++iCol; iCol < iColMax; iCol++)
		{
			uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow - Win_cen) + iCol + Win_cen;
			for (rw = 0; rw < Win_size; rw++, pRowIn += ar_cmmIn.m_i64LineSizeEl)
			{
				// Вычитаем из гистограммы столбец iCol - Win_cen - 1
				uint32_t iValue = pVarRow[iPos + rw];
				uint32_t iH = pHist[iValue]--;
				Sum += iValue * (1 - (iH << 1));
				// Добавим в гистограмму столбец iCol + Win_cen
				double d = (pRowIn[0] - pseudo_min) * kfct;
				if (d <= 0.)
					pVarRow[iPos + rw] = 0;
				else
				{
					int iVal = static_cast<int>(d + 0.5);
					if (iVal > 255)
					{
						iH = pHist[pVarRow[iPos + rw] = 255]++;
						Sum += 255 * (1 + (iH << 1));
					}
					else
					{
						iH = pHist[pVarRow[iPos + rw] = static_cast<uint8_t>(iVal)]++;
						Sum += iVal * (1 + (iH << 1));
					}
				}
			}
			iPos += Win_size;
			if (iPos >= Win_lin)
				iPos = 0;
			uint32_t Sum = 0;
			for (int i = 0; i < Win_lin; i++)
			{
				uint32_t iValue = static_cast<uint32_t>(pVarRow[i]);
				Sum += iValue * static_cast<uint32_t>(pHist[iValue]);
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
		}
		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}
	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pVarRow)
		delete[] pVarRow;
	if (nullptr != pHist)
		delete[] pHist;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V6(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int Win_size = (Win_cen << 1) + 1;
	int Win_lin = Win_size * Win_size;
	uint8_t *pVarRow = new uint8_t[Win_lin];
	uint16_t *pHist;
	double Size_obratn = 1.0 / Win_lin;
	// Кэш для гистограмм
	uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W];
	size_t uiHistSize = 256 * sizeof(uint16_t);
	// Кэш для сумм
	uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W];
	// Кэш для яркостей
	uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * (Win_size + 1)];
	uint8_t *pbBrightnessRow;
	int64_t iRow, iCol, iRowMax = ar_cmmIn.m_i64H - Win_cen, iColMax = ar_cmmIn.m_i64W - Win_cen;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int ln, rw, iPos = 0;
	uint32_t iValue, iH, Sum = 0;
	{ // Расчет кэшей, первая строка
		iRow = Win_cen;
		// Первая точка в строке
		iCol = Win_cen;
		pHist = pHistAll + 256 * iCol;
		memset(pHist, 0, uiHistSize);
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);
		{
			pbBrightnessRow = pbBrightness + iCol;
			for (rw = -Win_cen; rw <= Win_cen; rw++, pbBrightnessRow += ar_cmmIn.m_i64W)
			{
				uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow + rw);
				for (ln = -Win_cen; ln <= Win_cen; ln++)
				{
					double d = (pRowIn[iCol + ln] - pseudo_min) * kfct;
					if (d < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но pVarRow нужно обнулить!
						pbBrightnessRow[ln] = pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] = 0;
					else
					{
						if ((iValue = static_cast<uint32_t>(d + 0.5)) > 255)
						{
							pbBrightnessRow[ln] = pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] = 255;
							pHist[255]++;
						}
						else
						{
							pbBrightnessRow[ln] = pVarRow[(ln + Win_cen) * Win_size + (rw + Win_cen)] =
								static_cast<uint8_t>(iValue);
							pHist[iValue]++;
						}
					}
				}
			}
			for (int i = 0; i < Win_lin; i++)
			{
				iValue = static_cast<uint32_t>(pVarRow[i]);
				Sum += iValue * static_cast<uint32_t>(pHist[iValue]);
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
			puiSum[iCol] = Sum;
		}
		// Следующие точки в строке
		for (++iCol; iCol < iColMax; iCol++)
		{
			// Копируем гистограммы
			pHist = pHistAll + 256 * iCol;
			memcpy(pHist, pHist - 256, uiHistSize);
			uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow - Win_cen) + iCol + Win_cen;
			pbBrightnessRow = pbBrightness + iCol + Win_cen;
			for (rw = 0; rw < Win_size; rw++, pRowIn += ar_cmmIn.m_i64LineSizeEl, pbBrightnessRow += ar_cmmIn.m_i64W)
			{
				// Вычитаем из гистограммы столбец iCol - Win_cen - 1
				iH = pHist[iValue = pVarRow[iPos + rw]]--;
				Sum += iValue * (1 - (iH << 1));
				// Прибавляем в гистограмму столбец iCol + Win_cen
				double d = (pRowIn[0] - pseudo_min) * kfct;
				if (d < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но pVarRow нужно обнулить!
					pbBrightnessRow[0] = pVarRow[iPos + rw] = 0;
				else
				{
					if ((iValue = static_cast<uint32_t>(d + 0.5)) > 255)
					{
						pbBrightnessRow[0] = pVarRow[iPos + rw] = 255;
						iH = pHist[255]++;
						Sum += 255 * (1 + (iH << 1));
					}
					else
					{
						pbBrightnessRow[0] = pVarRow[iPos + rw] = static_cast<uint8_t>(iValue);
						iH = pHist[iValue]++;
						Sum += iValue * (1 + (iH << 1));
					}
				}
			}
			iPos += Win_size;
			if (iPos >= Win_lin)
				iPos = 0;
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
			puiSum[iCol] = Sum;
		}
		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
	}
	// Последующие строки
	int iBrRowSub = 0;		  // Из какой строки яркостей будем вычитать [0, Win_size]
	int iBrRowAdd = Win_size; // В какую строку яркостей будем заносить данные [0, Win_size]
	for (++iRow; iRow < iRowMax; iRow++)
	{
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// Обнуляем края. Левый край.
		memset(pfRowOut, 0, sizeof(float) * Win_cen);
		uint8_t *pbBrightnessRowSub = pbBrightness + iBrRowSub * ar_cmmIn.m_i64W;
		uint8_t *pbBrightnessRowAdd = pbBrightness + iBrRowAdd * ar_cmmIn.m_i64W;
		uint16_t *pRowInAdd = ar_cmmIn.pu16GetRow(iRow + Win_cen);
		// Первая точка в строке
		iCol = Win_cen;
		pHist = pHistAll + 256 * iCol;
		Sum = puiSum[iCol];
		{
			for (ln = 0; ln < Win_size; ln++)
			{
				// Вычитаем из гистограммы строку iRow - Win_cen - 1, колонки [iCol-Win_cen, iCol+Win_cen]
				iH = pHist[iValue = pbBrightnessRowSub[ln]]--;
				Sum += iValue * (1 - (iH << 1));
				// Прибавляем в гистограмму строку iRow + Win_cen, колонки [iCol-Win_cen, iCol+Win_cen]
				double d = (pRowInAdd[ln] - pseudo_min) * kfct;
				if (d < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но pVarRow нужно обнулить!
					pbBrightnessRowAdd[ln] = 0;
				else
				{
					if ((iValue = static_cast<uint32_t>(d + 0.5)) > 255)
					{
						pbBrightnessRowAdd[ln] = 255;
						iH = pHist[255]++;
						Sum += 255 * (1 + (iH << 1));
					}
					else
					{
						pbBrightnessRowAdd[ln] = static_cast<uint8_t>(iValue);
						iH = pHist[iValue]++;
						Sum += iValue * (1 + (iH << 1));
					}
				}
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
			puiSum[iCol] = Sum;
		}
		// Следующие точки в строке
		pbBrightnessRowSub++;
		pbBrightnessRowAdd++;
		pRowInAdd++;
		pHist += 256;
		for (++iCol; iCol < iColMax; iCol++, pbBrightnessRowSub++, pbBrightnessRowAdd++, pRowInAdd++, pHist += 256)
		{
			Sum = puiSum[iCol];
			for (ln = 0; ln < Win_size - 1; ln++)
			{ // Все данные есть в строке яркостей
				// Вычитаем из гистограммы строку iRow - Win_cen - 1, колонки [iCol-Win_cen, iCol+Win_cen]
				iH = pHist[iValue = pbBrightnessRowSub[ln]]--;
				Sum += iValue * (1 - (iH << 1));
				// Прибавляем в гистограмму строку iRow + Win_cen, колонки [iCol-Win_cen, iCol+Win_cen]
				iH = pHist[iValue = pbBrightnessRowAdd[ln]]++;
				Sum += iValue * (1 + (iH << 1));
			}
			{
				// Для добавляемой точки нужно рассчитать яркость
				// Вычитаем из гистограммы строку iRow - Win_cen - 1, колонки [iCol-Win_cen, iCol+Win_cen]
				iH = pHist[iValue = pbBrightnessRowSub[ln]]--;
				Sum += iValue * (1 - (iH << 1));
				// Прибавляем в гистограмму строку iRow + Win_cen, колонки [iCol-Win_cen, iCol+Win_cen]
				double d = (pRowInAdd[ln] - pseudo_min) * kfct;
				if (d < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но pVarRow нужно обнулить!
					pbBrightnessRowAdd[ln] = 0;
				else
				{
					if ((iValue = static_cast<uint32_t>(d + 0.5)) > 255)
					{
						pbBrightnessRowAdd[ln] = 255;
						iH = pHist[255]++;
						Sum += 255 * (1 + (iH << 1));
					}
					else
					{
						pbBrightnessRowAdd[ln] = static_cast<uint8_t>(iValue);
						iH = pHist[iValue]++;
						Sum += iValue * (1 + (iH << 1));
					}
				}
			}
			pfRowOut[iCol] = static_cast<float>(Sum * Size_obratn);
			puiSum[iCol] = Sum;
		}
		// Обнуляем края. Правый край.
		memset(pfRowOut + iColMax, 0, sizeof(float) * (ar_cmmOut.m_i64LineSizeEl - iColMax));
		++iBrRowSub;
		if (iBrRowSub > Win_size)
			iBrRowSub = 0;
		++iBrRowAdd;
		if (iBrRowAdd > Win_size)
			iBrRowAdd = 0;
	}
	// Обнуляем края. Последние строки.
	for (iRow = iRowMax; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pVarRow)
		delete[] pVarRow;
	if (nullptr != pHistAll)
		delete[] pHistAll;
	if (nullptr != puiSum)
		delete[] puiSum;
	if (nullptr != pbBrightness)
		delete[] pbBrightness;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V7(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int Win_size = (Win_cen << 1) + 1;
	int Win_lin = Win_size * Win_size;
	double Size_obratn = 1.0 / Win_lin;
	// Кэш для гистограмм
	uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
	memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
	uint16_t *pHist, *pHistAdd, *pHistSub;
	// Кэш для сумм
	uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
	memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);
	// Кэш для яркостей
	uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * Win_size];
	uint8_t *pbBrightnessRow;
	int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - Win_size;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int64_t i, iCmin, iCmax, iPos = 0;
	uint32_t u32ValueAdd, u32ValueSub;
	double d;
	// Первые [0, Win_size - 1] строки
	pbBrightnessRow = pbBrightness;
	float *pfRowOut = ar_cmmOut.pfGetRow(Win_cen) + Win_cen;
	for (iRow = 0; iRow < Win_size; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		for (; iCol <= iColMax; iCol++, puiSumCurr++)
		{
			iCmin = iCol - Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = 0; i < Win_size; i++)
					puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
				pfRowOut++;
			}
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
				pfRowOut++;
			}
		}
	}
	// Последующие строки [Win_size, ar_cmmIn.m_i64H[
	for (iRow = Win_size; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow - Win_cen) + Win_cen;
		iPos = (iRow - Win_size) % Win_size;
		pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		pHist = pHistAll + (iCol - Win_cen);
		for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
		{
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
																//	при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = 0; i < Win_size; i++)
					puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
									 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
			pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
			pfRowOut++;
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
			pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
			pfRowOut++;
		}
	}
	// Обнуляем края. Последние строки.
	for (iRow = ar_cmmIn.m_i64H - Win_cen; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pHistAll)
		delete[] pHistAll;
	if (nullptr != puiSum)
		delete[] puiSum;
	if (nullptr != pbBrightness)
		delete[] pbBrightness;
}

//================================================================
//================================================================

void TextureFiltr_Mean_V8(MU16Data &ar_cmmIn, MFData &ar_cmmOut, int Win_cen, double pseudo_min, double kfct)
{
	int Win_size = (Win_cen << 1) + 1;
	int Win_lin = Win_size * Win_size;
	double Size_obratn = 1.0 / Win_lin;
	// Кэш для гистограмм
	uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
	memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
	uint16_t *pHist, *pHistAdd, *pHistSub;
	// Кэш для сумм
	uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
	memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);
	// Кэш для яркостей
	uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * Win_size];
	uint8_t *pbBrightnessRow;
	int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - Win_size;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int64_t i, iCmin, iCmax, iPos = 0;
	uint32_t u32ValueAdd, u32ValueSub;
	double d;
	// Первые [0, Win_size - 1] строки
	pbBrightnessRow = pbBrightness;
	float *pfRowOut = ar_cmmOut.pfGetRow(Win_cen) + Win_cen;
	for (iRow = 0; iRow < Win_size; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		for (; iCol <= iColMax; iCol++, puiSumCurr++)
		{
			iCmin = iCol - Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = 0; i < Win_size; i++)
					puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
				pfRowOut++;
			}
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
				pfRowOut++;
			}
		}
	}
	// Последующие строки [Win_size, ar_cmmIn.m_i64H[
	for (iRow = Win_size; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow - Win_cen) + Win_cen;
		iPos = (iRow - Win_size) % Win_size;
		pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		pHist = pHistAll + (iCol - Win_cen);
		for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
		{
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
																//	при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;

				uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = 0; i < Win_size; i++)
					puiSumCurr[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) -
														u32ValueSub * static_cast<uint32_t>(pHistSub[i]--))
													   << 1);
			}
			pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
			pfRowOut++;
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
			pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
			pfRowOut++;
		}
	}
	// Обнуляем края. Последние строки.
	for (iRow = ar_cmmIn.m_i64H - Win_cen; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pHistAll)
		delete[] pHistAll;
	if (nullptr != puiSum)
		delete[] puiSum;
	if (nullptr != pbBrightness)
		delete[] pbBrightness;
}

//================================================================
//================================================================

/*
void TextureFiltr_Mean_V8_3(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)
{
	const int Win_cen = 1;
	int Win_size = (Win_cen << 1) + 1;
	int Win_lin = Win_size * Win_size;
	double Size_obratn = 1.0 / Win_lin;
	// Кэш для гистограмм
	uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
	memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
	uint16_t *pHist, *pHistAdd, *pHistSub;
	// Кэш для сумм
	uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
	memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);
	// Кэш для яркостей
	uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * Win_size];
	uint8_t *pbBrightnessRow;
	int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - Win_size;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int64_t i, iCmin, iCmax, iPos = 0;
	uint32_t u32ValueAdd, u32ValueSub;
	double d;
	// Первые [0, Win_size - 1] строки
	pbBrightnessRow = pbBrightness;
	float *pfRowOut = ar_cmmOut.pfGetRow(Win_cen) + Win_cen;
	for (iRow = 0; iRow < Win_size; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		for (; iCol <= iColMax; iCol++, puiSumCurr++)
		{
			iCmin = iCol - Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
				} // Добавляем яркость в Win_size гистограмм и сумм
				// for (i = 0; i < Win_size; i++)
				// 	puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
				puiSumCurr[0] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[0]++) << 1));
				puiSumCurr[1] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[1]++) << 1));
				puiSumCurr[2] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[2]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
				pfRowOut++;
			}
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
				pfRowOut++;
			}
		}
	}
	// Последующие строки [Win_size, ar_cmmIn.m_i64H[
	for (iRow = Win_size; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow - Win_cen) + Win_cen;
		iPos = (iRow - Win_size) % Win_size;
		pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		pHist = pHistAll + (iCol - Win_cen);
		for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
		{
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
																//	при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;

				uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;
				// Добавляем яркость в Win_size гистограмм и сумм
				// for (i = 0; i < Win_size; i++)
				// 	puiSumCurr[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) -
				// 										u32ValueSub * static_cast<uint32_t>(pHistSub[i]--))
				// 									   << 1);
				puiSumCurr[0] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[0]++) -
														u32ValueSub * static_cast<uint32_t>(pHistSub[0]--))
													   << 1);
				puiSumCurr[1] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[1]++) -
														u32ValueSub * static_cast<uint32_t>(pHistSub[1]--))
													   << 1);
				puiSumCurr[2] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[2]++) -
														u32ValueSub * static_cast<uint32_t>(pHistSub[2]--))
													   << 1);
			}
			pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
			pfRowOut++;
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
			pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
			pfRowOut++;
		}
	}
	// Обнуляем края. Последние строки.
	for (iRow = ar_cmmIn.m_i64H - Win_cen; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pHistAll)
		delete[] pHistAll;
	if (nullptr != puiSum)
		delete[] puiSum;
	if (nullptr != pbBrightness)
		delete[] pbBrightness;
}
*/

//================================================================
//================================================================

#include "../generated/TextureFiltr_Mean_V8_3_21.cpp"

//================================================================
//================================================================

void TextureFiltr_Mean_V8_3_sse(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)
{
#define Win_cen 2
#define Win_size ((Win_cen * 2) + 1)
#define Win_lin (Win_size * Win_size)
	double Size_obratn = 1.0 / Win_lin;
	// Кэш для гистограмм
	uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
	memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
	uint16_t *pHist, *pHistAdd, *pHistSub;
	// Кэш для сумм
	uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
	memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);
	// Кэш для яркостей
	uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * Win_size];
	uint8_t *pbBrightnessRow;
	int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - Win_size;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	int64_t i, iCmin, iCmax, iPos = 0;
	uint32_t u32ValueAdd, u32ValueSub;
	double d;
	// Первые [0, Win_size - 1] строки
	pbBrightnessRow = pbBrightness;
	float *pfRowOut = ar_cmmOut.pfGetRow(Win_cen) + Win_cen;
	for (iRow = 0; iRow < Win_size; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		for (; iCol <= iColMax; iCol++, puiSumCurr++)
		{
			iCmin = iCol - Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
				} // Добавляем яркость в Win_size гистограмм и сумм
				// for (i = 0; i < Win_size; i++)
				// 	puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
				puiSumCurr[0] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[0]++) << 1));
				puiSumCurr[1] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[1]++) << 1));
				puiSumCurr[2] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[2]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
				pfRowOut++;
			}
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				} // Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((Win_size - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
				pfRowOut++;
			}
		}
	}

	__m128i xmmHistOne_3 = _mm_set_epi16(0, 0, 0, 0, 0, 1, 1, 1); // Для inc или dec сразу для 3 гистограмм

	// Последующие строки [Win_size, ar_cmmIn.m_i64H[
	for (iRow = Win_size; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * Win_cen);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow - Win_cen) + Win_cen;
		iPos = (iRow - Win_size) % Win_size;
		pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);
		// Первые [0, 2*Win_cen[ колонки
		for (iCol = 0; iCol < (Win_cen << 1); iCol++)
		{
			iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
			iCmax = iCol + Win_cen;
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
		}
		// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
		puiSumCurr = puiSum + Win_cen;
		pHist = pHistAll + (iCol - Win_cen);
		for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
		{
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
																//	при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;

				uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;
				// Добавляем яркость в Win_size гистограмм и сумм
				//============================
				// Первые 3 точки
				//============================
				__m128i xmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub));   // Загружаем 128 бит (используем 48 бит - 3 эл. массива)
				__m128i xmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd));   // 3 элемента гистограммы
				__m128i xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr)); // Загружаем 128 бит (используем 96 бит - 3 эл. массива)
				__m128i xmmHistSub32 = _mm_cvtepu16_epi32(xmmHistSub);						   // HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
				__m128i xmmHistAdd32 = _mm_cvtepu16_epi32(xmmHistAdd);
				__m128i xmmValueAdd = _mm_loadu_si32(&u32ValueAdd); // 0, 0, 0, u32ValueAdd (Hi <-> Lo)
				__m128i xmmValueSub = _mm_loadu_si32(&u32ValueSub);
				xmmValueAdd = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueAdd),
															  _mm_castsi128_ps(xmmValueAdd), 0x0C0)); // 0, u32ValueAdd, u32ValueAdd, u32ValueAdd
				xmmValueSub = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueSub),
															  _mm_castsi128_ps(xmmValueSub), 0x0C0));

				xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd),
											 _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
				xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
				xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAdd);
				xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueSub);
				_mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr), _mm_add_epi32(xmmSumCurr,
																						xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]
				_mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd), _mm_add_epi16(xmmHistAdd,
																					  xmmHistOne_3)); // pHistAdd[0-2]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)
				_mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub), _mm_sub_epi16(xmmHistSub,
																					  xmmHistOne_3)); // pHistSub[0-2]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)
			}
			pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
			pfRowOut++;
		}
		// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - Win_cen;
			iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
				// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				// Добавляем яркость в Win_size гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
								 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
			}
			pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
			pfRowOut++;
		}
	}
	// Обнуляем края. Последние строки.
	for (iRow = ar_cmmIn.m_i64H - Win_cen; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
	if (nullptr != pHistAll)
		delete[] pHistAll;
	if (nullptr != puiSum)
		delete[] puiSum;
	if (nullptr != pbBrightness)
		delete[] pbBrightness;
#undef Win_cen
#undef Win_size
#undef Win_lin
}

//================================================================
//================================================================

#include "../generated/TextureFiltr_Mean_V8_sse4_3_21.cpp"

//================================================================
//================================================================

void TextureFiltr_Mean_V8_avx_21(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)
{
	double Size_obratn = 1.0 / 441;

	// Кэш для гистограмм
	uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
	memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
	uint16_t *pHist, *pHistAdd, *pHistSub;

	// Кэш для сумм
	uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
	memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);

	// Кэш для яркостей
	uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * 21];
	uint8_t *pbBrightnessRow;

	int64_t iRow, iCol, iColMax = ar_cmmIn.m_i64W - 21;
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);

	// Обнуляем края. Первые строки.
	for (iRow = 0; iRow < 10; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	int64_t i, iCmin, iCmax, iPos = 0;
	uint32_t u32ValueAdd, u32ValueSub;
	double d;

	// Первые [0, 21 - 1] строки
	pbBrightnessRow = pbBrightness;
	float *pfRowOut = ar_cmmOut.pfGetRow(10) + 10;
	for (iRow = 0; iRow < 21; iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * 10);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - 10), 0, sizeof(float) * 10);

		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);

		// Первые [0, 2*10[ колонки
		for (iCol = 0; iCol < (10 << 1); iCol++)
		{
			iCmin = std::max<int64_t>(10, iCol - 10);
			iCmax = iCol + 10;

			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				}
				// Добавляем яркость в 21 гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
		}

		// Средние [2*10, ar_cmmIn.m_i64W - 21] колонки
		puiSumCurr = puiSum + 10;
		for (; iCol <= iColMax; iCol++, puiSumCurr++)
		{
			iCmin = iCol - 10;

			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
				}
				// Добавляем яркость в 21 гистограмм и сумм
				for (i = 0; i < 21; i++)
					puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((21 - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
				pfRowOut++;
			}
		}

		// Последние [ar_cmmIn.m_i64W - 2*10, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - 10;
			iCmax = std::min(ar_cmmIn.m_i64W - 10 - 1, iCol + 10);

			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				pbBrightnessRow[iCol] = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
				{
					pbBrightnessRow[iCol] = u32ValueAdd = 255;
					pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
				}
				else
				{
					pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				}
				// Добавляем яркость в 21 гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
			}
			if ((21 - 1) == iRow)
			{
				pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
				pfRowOut++;
			}
		}
	}

	__m256i ymmHistOne_5 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1);   // Для inc или dec сразу для 5 гистограмм
	__m256i ymmHistOne_All = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1); // Для inc или dec сразу для всех гистограмм

	// Последующие строки [21, ar_cmmIn.m_i64H[
	for (iRow = 21; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		// Обнуляем края строк.
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * 10);
		memset(ar_cmmOut.pfGetRow(iRow) + (ar_cmmIn.m_i64W - 10), 0, sizeof(float) * 10);

		uint16_t *pRowIn = ar_cmmIn.pu16GetRow(iRow);
		float *pfRowOut = ar_cmmOut.pfGetRow(iRow - 10) + 10;
		iPos = (iRow - 21) % 21;
		pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);

		// Первые [0, 2*10[ колонки
		for (iCol = 0; iCol < (10 << 1); iCol++)
		{
			iCmin = std::max<int64_t>(10, iCol - 10);
			iCmax = iCol + 10;

			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;

				// Добавляем яркость в 21 гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) - u32ValueSub * static_cast<uint32_t>(pHistSub[i]--)) << 1);
			}
		}

		// Средние [2*10, ar_cmmIn.m_i64W - 21] колонки
		puiSumCurr = puiSum + 10;
		pHist = pHistAll + (iCol - 10);
		for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
		{
			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;

				// Добавляем яркость в 21 гистограмм и сумм
				//============================
				// Первые 16 точек
				//============================
				__m256i ymmHistSub = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(pHistSub)); // Загружаем 256 бит (используем 256 бит - 16 эл. массива)
				__m256i ymmHistAdd = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(pHistAdd));
				// Младшие 8 элементов гистограммы
				__m256i ymmSumCurr = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(puiSumCurr)); // Загружаем 256 бит (используем 256 бит - 8 эл. массива)
				__m256i ymmHistSub32 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(ymmHistSub)); // HistSub32[7], HistSub32[6], HistSub32[5], HistSub32[4], HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
				__m256i ymmHistAdd32 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(ymmHistAdd));
				__m256i ymmValueAdd = _mm256_castps_si256(_mm256_broadcast_ss(reinterpret_cast<float const *>(&u32ValueAdd))); // u32ValueAdd0, u32ValueAdd, u32ValueAdd, u32ValueAdd, u32ValueAdd0, u32ValueAdd, u32ValueAdd, u32ValueAdd (Hi <-> Lo)
				__m256i ymmValueSub = _mm256_castps_si256(_mm256_broadcast_ss(reinterpret_cast<float const *>(&u32ValueSub)));
				__m256i ymmValueAddSub = _mm256_add_epi32(ymmValueAdd, ymmValueSub);
				ymmHistAdd32 = _mm256_sub_epi32(_mm256_mullo_epi32(ymmHistAdd32, ymmValueAdd),
												_mm256_mullo_epi32(ymmHistSub32, ymmValueSub));
				ymmHistAdd32 = _mm256_add_epi32(ymmHistAdd32, ymmHistAdd32); // <<= 1
				ymmHistAdd32 = _mm256_add_epi32(ymmHistAdd32, ymmValueAddSub);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(puiSumCurr), _mm256_add_epi32(ymmSumCurr, ymmHistAdd32));
				// Сохраняем puiSumCurr[0,1,2,3,4,5,6,7]

				// Старшие 8 элементов гистограммы
				ymmSumCurr = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(puiSumCurr + 8));  // Загружаем 256 бит (используем 256 - 8 эл. массива)
				ymmHistSub32 = _mm256_cvtepu16_epi32(_mm256_extractf128_si256(ymmHistSub, 1)); // HistSub32[15], HistSub32[14], HistSub32[13], HistSub32[12], HistSub32[11], HistSub32[10], HistSub32[9], HistSub32[9] (Hi <-> Lo)
				ymmHistAdd32 = _mm256_cvtepu16_epi32(_mm256_extractf128_si256(ymmHistAdd, 1)); // Корректируем значение гистограммы
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(pHistAdd), _mm256_add_epi16(ymmHistAdd, ymmHistOne_All));
				// pHistAdd[0-15]++; Сохраняем 256 бит (используем 256 - 16 эл. массива)
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(pHistSub), _mm256_sub_epi16(ymmHistSub, ymmHistOne_All));
				// pHistSub[0-15]--; Сохраняем 256 бит (используем 256 - 16 эл. массива)
				ymmHistAdd32 = _mm256_sub_epi32(_mm256_mullo_epi32(ymmHistAdd32, ymmValueAdd),
												_mm256_mullo_epi32(ymmHistSub32, ymmValueSub));
				ymmHistAdd32 = _mm256_add_epi32(ymmHistAdd32, ymmHistAdd32); // <<= 1
				ymmHistAdd32 = _mm256_add_epi32(ymmHistAdd32, ymmValueAddSub);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(puiSumCurr + 8), _mm256_add_epi32(ymmSumCurr,
																								  ymmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3,4,5,6,7]
				//============================
				// Последние 5 точек
				//============================
				ymmHistSub = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(pHistSub + 16));   // Загружаем 256 бит (используем 80 бит - 5 эл. массива)
				ymmHistAdd = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(pHistAdd + 16));   // Младшие 5 элементов гистограммы
				ymmSumCurr = _mm256_lddqu_si256(reinterpret_cast<__m256i *>(puiSumCurr + 16)); // Загружаем 256 бит (используем 160 бит - 5 эл. массива)

				ymmHistSub32 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(ymmHistSub)); // HistSub32[7], HistSub32[6], HistSub32[5], HistSub32[4], HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
				ymmHistAdd32 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(ymmHistAdd));

				// Корректируем значение гистограммы
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(pHistAdd + 16), _mm256_add_epi16(ymmHistAdd, ymmHistOne_5));
				// pHistAdd[0-4]++; Сохраняем 256 бит (используем 256 - 16 эл. массива)
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(pHistSub + 16), _mm256_sub_epi16(ymmHistSub, ymmHistOne_5));
				// pHistSub[0-4]--; Сохраняем 256 бит (используем 256 - 16 эл. массива)
				ymmHistAdd = _mm256_setzero_si256();							  // обнулим все биты
				ymmValueAdd = _mm256_blend_epi32(ymmValueAdd, ymmHistAdd, 0x0E0); // 0, 0, 0, ymmValueAdd, ymmValueAdd, ymmValueAdd, ymmValueAdd, ymmValueAdd (Hi <-> Lo)
				ymmValueSub = _mm256_blend_epi32(ymmValueSub, ymmHistAdd, 0x0E0); // 0, 0, 0, ymmValueSub, ymmValueSub, ymmValueSub, ymmValueSub, ymmValueSub (Hi <-> Lo)
				ymmValueAddSub = _mm256_blend_epi32(ymmValueAddSub, ymmHistAdd, 0x0E0);
				ymmHistAdd32 = _mm256_sub_epi32(_mm256_mullo_epi32(ymmHistAdd32, ymmValueAdd),
												_mm256_mullo_epi32(ymmHistSub32, ymmValueSub));
				ymmHistAdd32 = _mm256_add_epi32(ymmHistAdd32, ymmHistAdd32); // <<= 1
				ymmHistAdd32 = _mm256_add_epi32(ymmHistAdd32, ymmValueAddSub);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(puiSumCurr + 16), _mm256_add_epi32(ymmSumCurr,
																								   ymmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3,4,5,6,7]
			}
			pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
			pfRowOut++;
		}

		// Последние [ar_cmmIn.m_i64W - 2*10, ar_cmmIn.m_i64W - 1] колонки
		for (; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			iCmin = iCol - 10;
			iCmax = std::min(ar_cmmIn.m_i64W - 10 - 1, iCol + 10);

			if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
				u32ValueAdd = 0;
			else
			{
				if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					u32ValueAdd = 255;
			}
			if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
			{
				pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
				pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
				pbBrightnessRow[iCol] = u32ValueAdd;
				uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;

				// Добавляем яркость в 21 гистограмм и сумм
				for (i = iCmin; i <= iCmax; i++)
					puiSum[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) - u32ValueSub * static_cast<uint32_t>(pHistSub[i]--)) << 1);
			}
			pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
			pfRowOut++;
		}
	}

	// Обнуляем края. Последние строки.
	for (iRow = ar_cmmIn.m_i64H - 10; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	if (nullptr != pHistAll)
		delete[] pHistAll;
	if (nullptr != puiSum)
		delete[] puiSum;
	if (nullptr != pbBrightness)
		delete[] pbBrightness;
}

//================================================================
//================================================================


void TextureFiltr_Mean_V8_3_sse_Omp(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)
{
#define Win_cen 1
#define Win_size ((Win_cen * 2) + 1)
#define Win_lin (Win_size * Win_size)
#define a_iThreads 1
	int64_t n_withoutBorderRows = ar_cmmIn.m_i64H - Win_cen * 2;
	int64_t fairRowsPerWorker = n_withoutBorderRows / a_iThreads;
	printf("ar_cmmIn.m_i64H %ld\n", ar_cmmIn.m_i64H);
	printf("n_withoutBorderRows %ld\n", n_withoutBorderRows);
	printf("fairRowsPerWorker %ld\n", fairRowsPerWorker);

	double Size_obratn = 1.0 / Win_lin;

	// Обнуляем края. Первые строки.
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
	for (int64_t iRow = 0; iRow < Win_cen; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

	// #pragma omp parallel for num_threads(a_iThreads)
	for (int iThread = 0; iThread < a_iThreads; iThread++)
	{
		// Кэш для гистограмм
		uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
		memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
		uint16_t *pHist, *pHistAdd, *pHistSub;

		// Кэш для сумм
		uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
		memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);

		// Кэш для яркостей
		uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * Win_size];
		uint8_t *pbBrightnessRow;
		int64_t iRow, g_iRow, iCol, iColMax = ar_cmmIn.m_i64W - Win_size;

		int64_t i, iCmin, iCmax, iPos = 0;
		uint32_t u32ValueAdd, u32ValueSub;
		double d;

		/*

		0 BBBBBBBBB
		1 S--------
		2
		3
		4 ---------
		5 S--------
		6
		7
		8 ---------
		9 BBBBBBBBB

		L = m_i64H - Win_cen * 2
		K = L / a_iThreads
		S(iThread) = K * iThread + Win_cen
		iRow_start = S(iThread) - Win_cen
		iRow_finish = S(iThread + 1) + Win_cen
		*/

		// int threadStartRow = iThread < a_iThreads ? fairRowsPerWorker * iThread + Win_cen : ar_cmmIn.m_i64H - Win_cen;
		int64_t g_StartRow = fairRowsPerWorker * iThread;
		int64_t g_EndRow = iThread + 1 == a_iThreads ? ar_cmmIn.m_i64H : g_StartRow + fairRowsPerWorker + Win_cen * 2;

		printf("\n\niThread %d\n", iThread);

		printf("g_StartRow %ld\n", g_StartRow);
		printf("g_EndRow %ld\n", g_EndRow);

		// Первые [0, Win_size - 1] строки
		pbBrightnessRow = pbBrightness;
		float *pfRowOut = ar_cmmOut.pfGetRow(g_StartRow + Win_cen) + Win_cen;

		for (iRow = 0, g_iRow = g_StartRow; iRow < Win_size; iRow++, g_iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
		{
			// Обнуляем края строк.
			memset(ar_cmmOut.pfGetRow(g_iRow), 0, sizeof(float) * Win_cen);
			memset(ar_cmmOut.pfGetRow(g_iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);
			uint16_t *pRowIn = ar_cmmIn.pu16GetRow(g_iRow);
			// Первые [0, 2*Win_cen[ колонки
			for (iCol = 0; iCol < (Win_cen << 1); iCol++)
			{
				iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
				iCmax = iCol + Win_cen;
				if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
					// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
					pbBrightnessRow[iCol] = 0;
				else
				{
					if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					{
						pbBrightnessRow[iCol] = u32ValueAdd = 255;
						pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
					}
					else
					{
						pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
						pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
					} // Добавляем яркость в Win_size гистограмм и сумм
					for (i = iCmin; i <= iCmax; i++)
						puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
				}
			}
			// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
			puiSumCurr = puiSum + Win_cen;
			for (; iCol <= iColMax; iCol++, puiSumCurr++)
			{
				iCmin = iCol - Win_cen;
				if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
					// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
					pbBrightnessRow[iCol] = 0;
				else
				{
					if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					{
						pbBrightnessRow[iCol] = u32ValueAdd = 255;
						pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
					}
					else
					{
						pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
						pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
					} // Добавляем яркость в Win_size гистограмм и сумм
					// for (i = 0; i < Win_size; i++)
					// 	puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
					puiSumCurr[0] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[0]++) << 1));
					puiSumCurr[1] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[1]++) << 1));
					puiSumCurr[2] += u32ValueAdd + u32ValueAdd * ((static_cast<uint32_t>(pHistAdd[2]++) << 1));
				}
				if ((Win_size - 1) == iRow)
				{
					pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
					pfRowOut++;
				}
			}
			// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
			for (; iCol < ar_cmmIn.m_i64W; iCol++)
			{
				iCmin = iCol - Win_cen;
				iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
				if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
					// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
					pbBrightnessRow[iCol] = 0;
				else
				{
					if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
					{
						pbBrightnessRow[iCol] = u32ValueAdd = 255;
						pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
					}
					else
					{
						pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
						pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
					} // Добавляем яркость в Win_size гистограмм и сумм
					for (i = iCmin; i <= iCmax; i++)
						puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
				}
				if ((Win_size - 1) == iRow)
				{
					pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
					pfRowOut++;
				}
			}
		}

		__m128i xmmHistOne_3 = _mm_set_epi16(0, 0, 0, 0, 0, 1, 1, 1); // Для inc или dec сразу для 3 гистограмм

		// Последующие строки [Win_size, ar_cmmIn.m_i64H[
		for (iRow = Win_size; g_iRow < g_EndRow; iRow++, g_iRow++)
		{
			// Обнуляем края строк.
			memset(ar_cmmOut.pfGetRow(g_iRow), 0, sizeof(float) * Win_cen);
			memset(ar_cmmOut.pfGetRow(g_iRow) + (ar_cmmIn.m_i64W - Win_cen), 0, sizeof(float) * Win_cen);

			uint16_t *pRowIn = ar_cmmIn.pu16GetRow(g_iRow);
			float *pfRowOut = ar_cmmOut.pfGetRow(g_iRow - Win_cen) + Win_cen;
			iPos = (iRow - Win_size) % Win_size;
			pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);

			// Первые [0, 2*Win_cen[ колонки
			for (iCol = 0; iCol < (Win_cen << 1); iCol++)
			{
				iCmin = std::max<int64_t>(Win_cen, iCol - Win_cen);
				iCmax = iCol + Win_cen;
				if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
					// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
					u32ValueAdd = 0;
				else
				{
					if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
						u32ValueAdd = 255;
				}
				if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
				{
					pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
					pbBrightnessRow[iCol] = u32ValueAdd;
					// Добавляем яркость в Win_size гистограмм и сумм
					for (i = iCmin; i <= iCmax; i++)
						puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
									 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
				}
			}
			// Средние [2*Win_cen, ar_cmmIn.m_i64W - Win_size] колонки
			puiSumCurr = puiSum + Win_cen;
			pHist = pHistAll + (iCol - Win_cen);
			for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
			{
				if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
																	//	при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
					u32ValueAdd = 0;
				else
				{
					if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
						u32ValueAdd = 255;
				}
				if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
				{
					pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
					pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
					pbBrightnessRow[iCol] = u32ValueAdd;

					uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;
					// Добавляем яркость в Win_size гистограмм и сумм
					//============================
					// Первые 3 точки
					//============================
					__m128i xmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub));   // Загружаем 128 бит (используем 48 бит - 3 эл. массива)
					__m128i xmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd));   // 3 элемента гистограммы
					__m128i xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr)); // Загружаем 128 бит (используем 96 бит - 3 эл. массива)
					__m128i xmmHistSub32 = _mm_cvtepu16_epi32(xmmHistSub);						   // HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
					__m128i xmmHistAdd32 = _mm_cvtepu16_epi32(xmmHistAdd);
					__m128i xmmValueAdd = _mm_loadu_si32(&u32ValueAdd); // 0, 0, 0, u32ValueAdd (Hi <-> Lo)
					__m128i xmmValueSub = _mm_loadu_si32(&u32ValueSub);
					xmmValueAdd = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueAdd),
																  _mm_castsi128_ps(xmmValueAdd), 0x0C0)); // 0, u32ValueAdd, u32ValueAdd, u32ValueAdd
					xmmValueSub = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueSub),
																  _mm_castsi128_ps(xmmValueSub), 0x0C0));

					xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd),
												 _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
					xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
					xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAdd);
					xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueSub);
					_mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr), _mm_add_epi32(xmmSumCurr,
																							xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]
					_mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd), _mm_add_epi16(xmmHistAdd,
																						  xmmHistOne_3)); // pHistAdd[0-2]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)
					_mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub), _mm_sub_epi16(xmmHistSub,
																						  xmmHistOne_3)); // pHistSub[0-2]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)
				}
				pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
				pfRowOut++;
			}
			// Последние [ar_cmmIn.m_i64W - 2*Win_cen, ar_cmmIn.m_i64W - 1] колонки
			for (; iCol < ar_cmmIn.m_i64W; iCol++)
			{
				iCmin = iCol - Win_cen;
				iCmax = std::min(ar_cmmIn.m_i64W - Win_cen - 1, iCol + Win_cen);
				if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0
					// при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
					u32ValueAdd = 0;
				else
				{
					if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
						u32ValueAdd = 255;
				}
				if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
				{
					pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
					pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
					pbBrightnessRow[iCol] = u32ValueAdd;
					// Добавляем яркость в Win_size гистограмм и сумм
					for (i = iCmin; i <= iCmax; i++)
						puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1)) -
									 u32ValueSub * ((static_cast<uint32_t>(pHistSub[i]--) << 1) - 1);
				}
				pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
				pfRowOut++;
			}
		}
		if (nullptr != pHistAll)
			delete[] pHistAll;
		if (nullptr != puiSum)
			delete[] puiSum;
		if (nullptr != pbBrightness)
			delete[] pbBrightness;
	}

	// Обнуляем края. Последние строки.
	for (int64_t iRow = ar_cmmIn.m_i64H - Win_cen; iRow < ar_cmmOut.m_i64H; iRow++)
		memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

#undef Win_cen
#undef Win_size
#undef Win_lin
#undef a_iThreads
}

//================================================================
//================================================================

#include "../generated/TextureFiltr_Mean_V8_sse4_Omp_3_21.cpp"

//================================================================
//================================================================

void TextureFiltr_Mean_V8_21_sse_Omp(MU16Data &ar_cmmIn, MFData &ar_cmmOut, double pseudo_min, double kfct)
{
#define a_iThreads 2
    int64_t n_withoutBorderRows = ar_cmmIn.m_i64H - 10 * 2;
    int64_t fairRowsPerWorker = n_withoutBorderRows / a_iThreads;
    double Size_obratn = 1.0 / 441;

    ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, 6);
    // Обнуляем края. Первые строки.
    for (int64_t iRow = 0; iRow < 10; iRow++)
        memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);

// #pragma omp parallel for num_threads(a_iThreads)
    for (int iThread = 0; iThread < a_iThreads; iThread++)
    {
        int64_t g_StartRow = fairRowsPerWorker * iThread;
        int64_t g_EndRow = iThread + 1 == a_iThreads ? ar_cmmIn.m_i64H : fairRowsPerWorker * (iThread + 1) + 10 * 2;

        // Кэш для гистограмм
        uint16_t *pHistAll = new uint16_t[256 * ar_cmmIn.m_i64W]; // [256][ar_cmmIn.m_i64W]
        memset(pHistAll, 0, sizeof(uint16_t) * 256 * ar_cmmIn.m_i64W);
        uint16_t *pHist, *pHistAdd, *pHistSub;

        // Кэш для сумм
        uint32_t *puiSum = new uint32_t[ar_cmmIn.m_i64W], *puiSumCurr;
        memset(puiSum, 0, sizeof(uint32_t) * ar_cmmIn.m_i64W);

        // Кэш для яркостей
        uint8_t *pbBrightness = new uint8_t[ar_cmmIn.m_i64W * 21];
        uint8_t *pbBrightnessRow;

        int64_t iRow, g_iRow, iCol, iColMax = ar_cmmIn.m_i64W - 21;

        int64_t i, iCmin, iCmax, iPos = 0;
        uint32_t u32ValueAdd, u32ValueSub;
        double d;

        // Первые [0, 21 - 1] строки
        pbBrightnessRow = pbBrightness;
        float *pfRowOut = ar_cmmOut.pfGetRow(g_StartRow + 10) + 10;
        for (iRow = 0, g_iRow = g_StartRow; iRow < 21; iRow++, g_iRow++, pbBrightnessRow += ar_cmmIn.m_i64W)
        {
            // Обнуляем края строк.
            memset(ar_cmmOut.pfGetRow(g_iRow), 0, sizeof(float) * 10);
            memset(ar_cmmOut.pfGetRow(g_iRow) + (ar_cmmIn.m_i64W - 10), 0, sizeof(float) * 10);

            uint16_t *pRowIn = ar_cmmIn.pu16GetRow(g_iRow);

            // Первые [0, 2*10[ колонки
            for (iCol = 0; iCol < (10 << 1); iCol++)
            {
                iCmin = std::max<int64_t>(10, iCol - 10);
                iCmax = iCol + 10;

                if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
                    pbBrightnessRow[iCol] = 0;
                else
                {
                    if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
                    {
                        pbBrightnessRow[iCol] = u32ValueAdd = 255;
                        pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
                    }
                    else
                    {
                        pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
                        pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
                    }
                    // Добавляем яркость в 21 гистограмм и сумм
                    for (i = iCmin; i <= iCmax; i++)
                        puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
                }
            }

            // Средние [2*10, ar_cmmIn.m_i64W - 21] колонки
            puiSumCurr = puiSum + 10;
            for (; iCol <= iColMax; iCol++, puiSumCurr++)
            {
                iCmin = iCol - 10;

                if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
                    pbBrightnessRow[iCol] = 0;
                else
                {
                    if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
                    {
                        pbBrightnessRow[iCol] = u32ValueAdd = 255;
                        pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W + iCmin);
                    }
                    else
                    {
                        pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
                        pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W + iCmin);
                    }
                    // Добавляем яркость в 21 гистограмм и сумм
                    for (i = 0; i < 21; i++)
                        puiSumCurr[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
                }
                if ((21 - 1) == iRow)
                {
                    pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
                    pfRowOut++;
                }
            }

            // Последние [ar_cmmIn.m_i64W - 2*10, ar_cmmIn.m_i64W - 1] колонки
            for (; iCol < ar_cmmIn.m_i64W; iCol++)
            {
                iCmin = iCol - 10;
                iCmax = std::min(ar_cmmIn.m_i64W - 10 - 1, iCol + 10);

                if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
                    pbBrightnessRow[iCol] = 0;
                else
                {
                    if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
                    {
                        pbBrightnessRow[iCol] = u32ValueAdd = 255;
                        pHistAdd = pHistAll + (255 * ar_cmmIn.m_i64W);
                    }
                    else
                    {
                        pbBrightnessRow[iCol] = static_cast<uint8_t>(u32ValueAdd);
                        pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
                    }
                    // Добавляем яркость в 21 гистограмм и сумм
                    for (i = iCmin; i <= iCmax; i++)
                        puiSum[i] += u32ValueAdd * (1 + (static_cast<uint32_t>(pHistAdd[i]++) << 1));
                }
                if ((21 - 1) == iRow)
                {
                    pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
                    pfRowOut++;
                }
            }
        }

        __m128i xmmHistOne_5 = _mm_set_epi16(0, 0, 0, 1, 1, 1, 1, 1);   // Для inc или dec сразу для 5 гистограмм
        __m128i xmmHistOne_All = _mm_set_epi16(1, 1, 1, 1, 1, 1, 1, 1); // Для inc или dec сразу для всех гистограмм

        // Последующие строки [21, ar_cmmIn.m_i64H[
        for (iRow = 21; g_iRow < g_EndRow; iRow++, g_iRow++)
        {
            // Обнуляем края строк.
            memset(ar_cmmOut.pfGetRow(g_iRow), 0, sizeof(float) * 10);
            memset(ar_cmmOut.pfGetRow(g_iRow) + (ar_cmmIn.m_i64W - 10), 0, sizeof(float) * 10);

            uint16_t *pRowIn = ar_cmmIn.pu16GetRow(g_iRow);
            float *pfRowOut = ar_cmmOut.pfGetRow(g_iRow - 10) + 10;
            iPos = (iRow - 21) % 21;
            pbBrightnessRow = pbBrightness + (iPos * ar_cmmIn.m_i64W);

            // Первые [0, 2*10[ колонки
            for (iCol = 0; iCol < (10 << 1); iCol++)
            {
                iCmin = std::max<int64_t>(10, iCol - 10);
                iCmax = iCol + 10;

                if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
                    u32ValueAdd = 0;
                else
                {
                    if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
                        u32ValueAdd = 255;
                }
                if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
                {
                    pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
                    pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
                    pbBrightnessRow[iCol] = u32ValueAdd;
                    uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;

                    // Добавляем яркость в 21 гистограмм и сумм
                    for (i = iCmin; i <= iCmax; i++)
                        puiSum[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) - u32ValueSub * static_cast<uint32_t>(pHistSub[i]--)) << 1);
                }
            }

            // Средние [2*10, ar_cmmIn.m_i64W - 21] колонки
            puiSumCurr = puiSum + 10;
            pHist = pHistAll + (iCol - 10);
            for (; iCol <= iColMax; iCol++, puiSumCurr++, pHist++)
            {
                if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
                    u32ValueAdd = 0;
                else
                {
                    if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
                        u32ValueAdd = 255;
                }
                if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
                {
                    pHistSub = pHist + (u32ValueSub * ar_cmmIn.m_i64W);
                    pHistAdd = pHist + (u32ValueAdd * ar_cmmIn.m_i64W);
                    pbBrightnessRow[iCol] = u32ValueAdd;

                    // Добавляем яркость в 21 гистограмм и сумм
                    //============================
                    // Первые 8 точек
                    //============================
                    __m128i xmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub)); // Загружаем 128 бит (используем 128 бит - 8 эл. массива)
                    __m128i xmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd));

                    // Младшие 4 элемента гистограммы
                    __m128i xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr)); // Загружаем 128 бит (используем 128 бит - 4 эл. массива)

                    __m128i xmmHistSub32 = _mm_cvtepu16_epi32(xmmHistSub); // HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
                    __m128i xmmHistAdd32 = _mm_cvtepu16_epi32(xmmHistAdd);

                    __m128i xmmValueAdd = _mm_loadu_si32(&u32ValueAdd); // 0, 0, 0, u32ValueAdd (Hi <-> Lo)
                    __m128i xmmValueSub = _mm_loadu_si32(&u32ValueSub);

                    xmmValueAdd = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueAdd), _mm_castsi128_ps(xmmValueAdd), 0x0)); // 0, u32ValueAdd, u32ValueAdd, u32ValueAdd
                    xmmValueSub = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(xmmValueSub), _mm_castsi128_ps(xmmValueSub), 0x0));

                    __m128i xmmValueAddSub = _mm_add_epi32(xmmValueAdd, xmmValueSub);

                    xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);

                    _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]

                    // Старшие 4 элемента гистограммы
                    xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 4)); // Загружаем 128 бит (используем 128 - 4 эл. массива)

                    xmmHistSub32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistSub, 8)); // HistSub32[7], HistSub32[6], HistSub32[5], HistSub32[4] (Hi <-> Lo)
                    xmmHistAdd32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistAdd, 8));

                    // Корректируем значение гистограммы
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd), _mm_add_epi16(xmmHistAdd, xmmHistOne_All)); // pHistAdd[0-7]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub), _mm_sub_epi16(xmmHistSub, xmmHistOne_All)); // pHistSub[0-7]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)

                    xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);

                    _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 4), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]
                    //============================
                    // Вторые 8 точек
                    //============================
                    xmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub + 8)); // Загружаем 128 бит (используем 128 бит - 8 эл. массива)
                    xmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd + 8));

                    // Младшие 4 элемента гистограммы
                    xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 8)); // Загружаем 128 бит (используем 128 бит - 4 эл. массива)

                    xmmHistSub32 = _mm_cvtepu16_epi32(xmmHistSub); // HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
                    xmmHistAdd32 = _mm_cvtepu16_epi32(xmmHistAdd);

                    xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);

                    _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 8), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]

                    // Старшие 4 элемента гистограммы
                    xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 12)); // Загружаем 128 бит (используем 128 - 4 эл. массива)

                    xmmHistSub32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistSub, 8)); // HistSub32[7], HistSub32[6], HistSub32[5], HistSub32[4] (Hi <-> Lo)
                    xmmHistAdd32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistAdd, 8));

                    // Корректируем значение гистограммы
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd + 8), _mm_add_epi16(xmmHistAdd, xmmHistOne_All)); // pHistAdd[0-7]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub + 8), _mm_sub_epi16(xmmHistSub, xmmHistOne_All)); // pHistSub[0-7]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)

                    xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);

                    _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 12), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]
                    //============================
                    // Последние 5 точек
                    //============================
                    xmmHistSub = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistSub + 16)); // Загружаем 128 бит (используем 80 бит - 5 эл. массива)
                    xmmHistAdd = _mm_lddqu_si128(reinterpret_cast<__m128i *>(pHistAdd + 16));

                    // Младшие 4 элемента гистограммы
                    xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 16)); // Загружаем 128 бит (используем 128 бит - 4 эл. массива)

                    xmmHistSub32 = _mm_cvtepu16_epi32(xmmHistSub); // HistSub32[3], HistSub32[2], HistSub32[1], HistSub32[0] (Hi <-> Lo)
                    xmmHistAdd32 = _mm_cvtepu16_epi32(xmmHistAdd);

                    xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);

                    _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 16), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]

                    // Старший 1 элемент гистограммы
                    xmmSumCurr = _mm_lddqu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 20)); // Загружаем 128 бит (используем 32 - 1 эл. массива)

                    xmmHistSub32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistSub, 8)); // HistSub32[7], HistSub32[6], HistSub32[5], HistSub32[4] (Hi <-> Lo)
                    xmmHistAdd32 = _mm_cvtepu16_epi32(_mm_bsrli_si128(xmmHistAdd, 8));

                    // Корректируем значение гистограммы
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistAdd + 16), _mm_add_epi16(xmmHistAdd, xmmHistOne_5)); // pHistAdd[0-4]++; Сохраняем 128 бит (используем 128 - 8 эл. массива)
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(pHistSub + 16), _mm_sub_epi16(xmmHistSub, xmmHistOne_5)); // pHistSub[0-4]--; Сохраняем 128 бит (используем 128 - 8 эл. массива)

                    xmmValueAdd = _mm_bsrli_si128(xmmValueAdd, 12); // 0, 0, 0, xmmValueAdd (Hi <-> Lo)
                    xmmValueSub = _mm_bsrli_si128(xmmValueSub, 12); // 0, 0, 0, xmmValueSub (Hi <-> Lo)
                    xmmValueAddSub = _mm_bsrli_si128(xmmValueAddSub, 12);

                    xmmHistAdd32 = _mm_sub_epi32(_mm_mullo_epi32(xmmHistAdd32, xmmValueAdd), _mm_mullo_epi32(xmmHistSub32, xmmValueSub));
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmHistAdd32); // <<= 1
                    xmmHistAdd32 = _mm_add_epi32(xmmHistAdd32, xmmValueAddSub);

                    _mm_storeu_si128(reinterpret_cast<__m128i *>(puiSumCurr + 20), _mm_add_epi32(xmmSumCurr, xmmHistAdd32)); // Сохраняем puiSumCurr[0,1,2,3]
                }
                pfRowOut[0] = static_cast<float>(puiSumCurr[0] * Size_obratn);
                pfRowOut++;
            }

            // Последние [ar_cmmIn.m_i64W - 2*10, ar_cmmIn.m_i64W - 1] колонки
            for (; iCol < ar_cmmIn.m_i64W; iCol++)
            {
                iCmin = iCol - 10;
                iCmax = std::min(ar_cmmIn.m_i64W - 10 - 1, iCol + 10);

                if ((d = (pRowIn[iCol] - pseudo_min) * kfct) < 0.5) // Так как при яркости 0 вклад в сумму будет 0 при любом pHist[0], то pHist[0] можно просто нигде не учитывать, но яркость в кэше нужно обнулить!
                    u32ValueAdd = 0;
                else
                {
                    if ((u32ValueAdd = static_cast<uint32_t>(d + 0.5)) > 255)
                        u32ValueAdd = 255;
                }
                if (u32ValueAdd != (u32ValueSub = pbBrightnessRow[iCol]))
                {
                    pHistSub = pHistAll + (u32ValueSub * ar_cmmIn.m_i64W);
                    pHistAdd = pHistAll + (u32ValueAdd * ar_cmmIn.m_i64W);
                    pbBrightnessRow[iCol] = u32ValueAdd;
                    uint32_t u32ValueAddSub = u32ValueAdd + u32ValueSub;

                    // Добавляем яркость в 21 гистограмм и сумм
                    for (i = iCmin; i <= iCmax; i++)
                        puiSum[i] += u32ValueAddSub + ((u32ValueAdd * static_cast<uint32_t>(pHistAdd[i]++) - u32ValueSub * static_cast<uint32_t>(pHistSub[i]--)) << 1);
                }
                pfRowOut[0] = static_cast<float>(puiSum[iCmin] * Size_obratn);
                pfRowOut++;
            }
        }

        if (nullptr != pHistAll)
            delete[] pHistAll;
        if (nullptr != puiSum)
            delete[] puiSum;
        if (nullptr != pbBrightness)
            delete[] pbBrightness;
    }
    // Обнуляем края. Последние строки.
    for (int64_t iRow = ar_cmmIn.m_i64H - 10; iRow < ar_cmmOut.m_i64H; iRow++)
        memset(ar_cmmOut.pfGetRow(iRow), 0, sizeof(float) * ar_cmmOut.m_i64LineSizeEl);
}


//================================================================
//================================================================

#define FUNC() TextureFiltr_Mean_V8_sse4_Omp_3_21
#define THREADS 8
// #define ONE_WIN_CEN 10

int mainTestTextureFiltr_Mean()
{

	const char *dataDirPath = "../data/";
	const char *outDirPath = "./";
	const char *pcFileNameIn = "TextureFiltrIn.mu16";
	const char *referenceDirPath = "../data/reference/";
	std::string strFileIn, strFileOut;

	MU16Data m;
	MFData mOut;
	int Win_cen = 1;
	double pseudo_min, kfct;

	strFileIn = dataDirPath;
	strFileIn += pcFileNameIn;
	m.iRead(strFileIn.c_str());

	// Найдем мин и мах
	uint16_t u16Min, u16Max;
	u16Min = u16Max = m.pu16GetRow(0)[0];
	for (int64_t iRow = 0; iRow < m.m_i64H; iRow++)
	{
		uint16_t *pRowIn = m.pu16GetRow(iRow);
		for (int64_t iCol = 0; iCol < m.m_i64W; iCol++)
		{
			if (u16Min > pRowIn[iCol])
				u16Min = pRowIn[iCol];
			else if (u16Max < pRowIn[iCol])
				u16Max = pRowIn[iCol];
		}
	}
	// printf("%ldx%ld\n", m.m_i64W, m.m_i64H);
	// return 0;

	pseudo_min = u16Min;
	kfct = 255. / (u16Max - u16Min);

	std::string strDebug;
	strDebug = outDirPath;
	strDebug += "Trace.txt";
	FILE *pf = fopen(strDebug.c_str(), "a+t");
	if (nullptr == pf)
		return -1;

	std::string strTab, strValue, strPoint("."), strComma(",");
	strTab = outDirPath;
	strTab += "TraceTab.md";
	FILE *pfTab = fopen(strTab.c_str(), "a+t");
	if (nullptr == pfTab)
		return -2;

	double dStart, dEnd;

#define _STRINGIZE(x) #x
#define GET_STRING(x) _STRINGIZE(x)

	std::string strFunName = GET_STRING(FUNC());
	fprintf(pf, "\n%s\n", strFunName.c_str());
	printf("\n%s\n", strFunName.c_str());
#ifdef THREADS
	for (int iThreads = 1; iThreads <= THREADS; iThreads <<= 1)
#else
	for (int iThreads = 1; iThreads <= 1; iThreads <<= 1)
#endif
	{
		fprintf(pf, "%d\n", iThreads);
		printf("%d Threads\n", iThreads);
		fprintf(pfTab, "|%s|%d", strFunName.c_str(), iThreads);

#ifdef ONE_WIN_CEN
		for (Win_cen = ONE_WIN_CEN; Win_cen <= ONE_WIN_CEN; Win_cen++)
#else
		for (Win_cen = 1; Win_cen <= 10; Win_cen++)
#endif
		{
			int Win_size = 2 * Win_cen + 1;
			dStart = omp_get_wtime();
#if defined(THREADS)
			FUNC()
			(m, mOut, Win_cen, pseudo_min, kfct, iThreads);
#elif defined(ONE_WIN_CEN)
			FUNC()
			(m, mOut, pseudo_min, kfct);
#else
			FUNC()
			(m, mOut, Win_cen, pseudo_min, kfct);
#endif
			dEnd = omp_get_wtime();
			strFileOut = outDirPath + std::string("Out") + std::to_string(Win_size) + "x" +
						 std::to_string(Win_size) + "_new.mfd";
			mOut.iWrite(strFileOut.c_str());
			printf("%18s - %10lg ", strFileOut.c_str(), dEnd - dStart);
			fprintf(pf, "%s - %lg\n", strFileOut.c_str(), dEnd - dStart);

			// Запись в таблицу
			// strValue = std::to_string(dEnd - dStart); // double -> string
			// size_t pos = strValue.find(strPoint); // '.' -> ','
			// while (pos != std::string::npos) {
			// 	strValue.replace(pos, strComma.size(), strComma);
			// 	pos = strValue.find(strPoint, pos);
			// }

			fprintf(pfTab, "|%0.2lf", dEnd - dStart);

			{ // Сравнение результатов, полученных при применении исходного алгоритма и ускоренного
				MFData mSrc;
				std::string strFileTest = referenceDirPath + std::string("Out") + std::to_string(Win_size) + "x" +
										  std::to_string(Win_size) + "_src.mfd";
				if (0 != mSrc.iRead(strFileTest.c_str()))
				{
					printf("Не удалось прочитать файл '%s'\n", strFileTest.c_str());
					fprintf(pf, "Не удалось прочитать файл '%s'\n", strFileTest.c_str());
					fclose(pf);
					return -1;
				}
				float percentWrong;
				int iRet = iCompareDataF32(mSrc, mOut, percentWrong);

				if (0 != iRet)
				{
					printf("Percent wrong: %0.3f%%\n", percentWrong * 100);
					fprintf(pf, "Percent wrong: %0.3f%%\n", percentWrong * 100);
				}
				else
				{
					printf("EQ!\n");
					// fprintf(pf, "EQ!\n");
				}
			}
		}
		fprintf(pfTab, "|\n");
		fflush(pfTab);
	}
	fclose(pf);
	fclose(pfTab);
	return 0;
}

int mainGenerateTest5x5()
{
	MU16Data m;
	m.iCreate(5, 5, 1);
	int iNum = 0;
	for (int iRow = 0; iRow < 5; iRow++)
	{
		for (int iCol = 0; iCol < 5; iCol++)
		{
			m.m_ppRows[iRow][iCol] = iNum++;
		}
	}
	m.m_ppRows[4][4] = 255;
	m.iWrite("../data/test5x5.mu16");
	return 0;
}

int mainOriginal()
{
	const char *pcFilePath = "./";
	const char *pcFileNameIn = "TextureFiltrIn.mu16";
	std::string strFileIn, strFileOut;

	MU16Data m;
	MFData mOut;
	int Win_cen = 1;
	int Win_size = 2 * Win_cen + 1;
	double pseudo_min, kfct;

	strFileIn = pcFilePath;
	strFileIn += pcFileNameIn;
	m.iRead(strFileIn.c_str());

	// Найдем мин и мах
	uint16_t u16Min, u16Max;
	u16Min = u16Max = m.pu16GetRow(0)[0];
	for (int64_t iRow = 0; iRow < m.m_i64H; iRow++)
	{
		uint16_t *pRowIn = m.pu16GetRow(iRow);
		for (int64_t iCol = 0; iCol < m.m_i64W; iCol++)
		{
			if (u16Min > pRowIn[iCol])
				u16Min = pRowIn[iCol];
			else if (u16Max < pRowIn[iCol])
				u16Max = pRowIn[iCol];
		}
	}

	pseudo_min = u16Min;
	kfct = 255. / (u16Max - u16Min);

	TextureFiltr_Mean(m, mOut, Win_cen, pseudo_min, kfct);

	strFileOut = pcFilePath + std::string("Out") + std::to_string(Win_size) + "x" + std::to_string(Win_size) + "_src.mfd";
	mOut.iWrite(strFileOut.c_str());

	return 0;
}

int main()
{
	return mainTestTextureFiltr_Mean();
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

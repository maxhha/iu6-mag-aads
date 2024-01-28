// ConsoleApplication1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <conio.h>
#include <string>
#include <omp.h>

#include "MU16Data.h"

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
					double d = (pRowIn[iCol + ln] - pseudo_min)*kfct;
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

int main()
{
	const char *pcFilePath = "Z:/505/ИУ6-14М/Щугорев/TextureFiltr/";
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

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

#include "MU16Data.h"

//================================================================
// class MU16Data
//================================================================
MU16Data::MU16Data()
	: m_i64W(0),
	m_i64H(0),
	m_i64Pow(0),
	m_i64LineSizeEl(0),
	m_pData(nullptr),
	m_ppRows(nullptr)
{
}

MU16Data::~MU16Data()
{
	deleteData();
}

void MU16Data::deleteData()
{
	if (nullptr != m_ppRows)
	{
		delete[] m_ppRows;
		m_ppRows = nullptr;
	}

	if (nullptr != m_pData)
	{
		delete[] m_pData;
		m_pData = nullptr;
	}
	m_i64W = 0;
	m_i64H = 0;
	m_i64Pow = 0;
	m_i64LineSizeEl = 0;
}

int MU16Data::iRead(const char *a_pcFileName)
{
	deleteData();

	FILE *pf;
	int iRet = fopen_s(&pf, a_pcFileName, "rb");
	if (0 != iRet)
		return iRet;

	while (true)
	{
		if (fread(&m_i64W, sizeof(int64_t), 4, pf) != 4)
		{
			iRet = -2;
			break;
		}

		// Отводим память
		int64_t i64Align = static_cast<int64_t>(1) << m_i64Pow, i64;
		int64_t i64CountEl = m_i64LineSizeEl * m_i64H;
		if (i64Align > 4)
			i64CountEl += ((i64Align << 1) >> 2);
		else
			i64CountEl++;
		m_pData = new int8_t[i64CountEl * sizeof(uint16_t)];
		m_ppRows = new uint16_t*[m_i64H];

		// Выравниваем и заполняем массив строк
		unionPtrToInt64 ui;
		ui.m_pv = m_pData;
		ui.m_i64 += (i64Align - (ui.m_i64 & (i64Align - 1))) & (i64Align - 1);
		m_ppRows[0] = static_cast<uint16_t *>(ui.m_pv);
		for (i64 = 1; i64 < m_i64H; i64++)
			m_ppRows[i64] = m_ppRows[i64 - 1] + m_i64LineSizeEl;

		// Читаем собственно данные
		if (fread(pu16GetRow(0), sizeof(uint16_t), m_i64LineSizeEl*m_i64H, pf) != (m_i64LineSizeEl*m_i64H))
		{
			iRet = -3;
			break;
		}
		break;
	}
	fclose(pf);
	return iRet;
}

int MU16Data::iCreate(int64_t a_i64W, int64_t a_i64H, int64_t a_i64Pow)
{
	deleteData();

	m_i64W = a_i64W;
	m_i64H = a_i64H;
	m_i64Pow = a_i64Pow;

	int64_t i64Align = static_cast<int64_t>(1) << m_i64Pow, i64;
	m_i64LineSizeEl = (m_i64W << 2);				// Переведем в байты
	m_i64LineSizeEl += (i64Align - (m_i64LineSizeEl & (i64Align - 1))) & (i64Align - 1);
	m_i64LineSizeEl >>= 2;							// Переведем в элементы

	// Отводим память
	int64_t i64CountEl = m_i64LineSizeEl * m_i64H;
	if (i64Align > 4)
		i64CountEl += ((i64Align << 1) >> 2);
	else
		i64CountEl++;
	m_pData = new int8_t[i64CountEl * sizeof(uint16_t)];
	m_ppRows = new uint16_t*[m_i64H];

	// Выравниваем и заполняем массив строк
	unionPtrToInt64 ui;
	ui.m_pv = m_pData;
	ui.m_i64 += (i64Align - (ui.m_i64 & (i64Align - 1))) & (i64Align - 1);
	m_ppRows[0] = static_cast<uint16_t *>(ui.m_pv);
	for (i64 = 1; i64 < m_i64H; i64++)
		m_ppRows[i64] = m_ppRows[i64 - 1] + m_i64LineSizeEl;

	return 0;
}

int MU16Data::iWrite(const char *a_pcFileName)
{
	FILE *pf;
	int iRet = fopen_s(&pf, a_pcFileName, "wb");
	if (0 != iRet)
		return iRet;

	while (true)
	{
		if (fwrite(&m_i64W, sizeof(int64_t), 4, pf) != 4)
		{
			iRet = -2;
			break;
		}

		// Пишем собственно данные
		if (fwrite(pu16GetRow(0), sizeof(uint16_t), m_i64LineSizeEl*m_i64H, pf) != (m_i64LineSizeEl*m_i64H))
		{
			iRet = -3;
			break;
		}
		break;
	}
	fclose(pf);
	return iRet;
}

//================================================================
//================================================================

//================================================================
// class MFData
//================================================================
MFData::MFData()
	: m_i64W(0),
	m_i64H(0),
	m_i64Pow(0),
	m_i64LineSizeEl(0),
	m_pData(nullptr),
	m_ppRows(nullptr)
{
}

MFData::~MFData()
{
	deleteData();
}

void MFData::deleteData()
{
	if (nullptr != m_ppRows)
	{
		delete[] m_ppRows;
		m_ppRows = nullptr;
	}

	if (nullptr != m_pData)
	{
		delete[] m_pData;
		m_pData = nullptr;
	}
	m_i64W = 0;
	m_i64H = 0;
	m_i64Pow = 0;
	m_i64LineSizeEl = 0;
}

int MFData::iRead(const char *a_pcFileName)
{
	deleteData();

	FILE *pf;
	int iRet = fopen_s(&pf, a_pcFileName, "rb");
	if (0 != iRet)
		return iRet;

	while (true)
	{
		if (fread(&m_i64W, sizeof(int64_t), 4, pf) != 4)
		{
			iRet = -2;
			break;
		}

		// Отводим память
		int64_t i64Align = static_cast<int64_t>(1) << m_i64Pow, i64;
		int64_t i64CountEl = m_i64LineSizeEl * m_i64H;
		if (i64Align > 4)
			i64CountEl += ((i64Align << 1) >> 2);
		else
			i64CountEl++;
		m_pData = new int8_t[i64CountEl * sizeof(float)];
		m_ppRows = new float*[m_i64H];

		// Выравниваем и заполняем массив строк
		unionPtrToInt64 ui;
		ui.m_pv = m_pData;
		ui.m_i64 += (i64Align - (ui.m_i64 & (i64Align - 1))) & (i64Align - 1);
		m_ppRows[0] = static_cast<float *>(ui.m_pv);
		for (i64 = 1; i64 < m_i64H; i64++)
			m_ppRows[i64] = m_ppRows[i64 - 1] + m_i64LineSizeEl;

		// Читаем собственно данные
		if (fread(pfGetRow(0), sizeof(float), m_i64LineSizeEl*m_i64H, pf) != (m_i64LineSizeEl*m_i64H))
		{
			iRet = -3;
			break;
		}
		break;
	}
	fclose(pf);
	return iRet;
}

int MFData::iCreate(int64_t a_i64W, int64_t a_i64H, int64_t a_i64Pow)
{
	deleteData();

	m_i64W = a_i64W;
	m_i64H = a_i64H;
	m_i64Pow = a_i64Pow;

	int64_t i64Align = static_cast<int64_t>(1) << m_i64Pow, i64;
	m_i64LineSizeEl = (m_i64W << 2);					// Переведем в байты
	m_i64LineSizeEl += (i64Align - (m_i64LineSizeEl & (i64Align - 1))) & (i64Align - 1);
	m_i64LineSizeEl >>= 2;							// Переведем в элементы

	// Отводим память
	int64_t i64CountEl = m_i64LineSizeEl * m_i64H;
	if (i64Align > 4)
		i64CountEl += ((i64Align << 1) >> 2);
	else
		i64CountEl++;
	m_pData = new int8_t[i64CountEl * sizeof(float)];
	m_ppRows = new float*[m_i64H];

	// Выравниваем и заполняем массив строк
	unionPtrToInt64 ui;
	ui.m_pv = m_pData;
	ui.m_i64 += (i64Align - (ui.m_i64 & (i64Align - 1))) & (i64Align - 1);
	m_ppRows[0] = static_cast<float *>(ui.m_pv);
	for (i64 = 1; i64 < m_i64H; i64++)
		m_ppRows[i64] = m_ppRows[i64 - 1] + m_i64LineSizeEl;

	return 0;
}

int MFData::iWrite(const char *a_pcFileName)
{
	FILE *pf;
	int iRet = fopen_s(&pf, a_pcFileName, "wb");
	if (0 != iRet)
		return iRet;

	while (true)
	{
		if (fwrite(&m_i64W, sizeof(int64_t), 4, pf) != 4)
		{
			iRet = -2;
			break;
		}

		// Пишем собственно данные
		if (fwrite(pfGetRow(0), sizeof(float), m_i64LineSizeEl*m_i64H, pf) != (m_i64LineSizeEl*m_i64H))
		{
			iRet = -3;
			break;
		}
		break;
	}
	fclose(pf);
	return iRet;
}

//================================================================
//================================================================

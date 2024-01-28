#pragma once

#include <stdio.h>
#include <stdint.h>

typedef union
{
	void*       m_pv;
	int64_t     m_i64;
} unionPtrToInt64;

//================================================================
// class MU16Data
//================================================================
class MU16Data
{
public:
	MU16Data();
	~MU16Data();
	void deleteData();

	int iRead(const char *a_pcFileName);
	int iCreate(int64_t a_i64W, int64_t a_i64H, int64_t a_i64Pow);
	int iWrite(const char *a_pcFileName);

	inline uint16_t *pu16GetRow(int64_t a_i64Row) { return m_ppRows[a_i64Row]; }

public:
	int64_t	m_i64W;				// Ширина
	int64_t	m_i64H;				// Высота
	int64_t	m_i64Pow;			// Выравнивание (степень 2)
	int64_t	m_i64LineSizeEl;		// Длина строки в элементах, с учетом выравнивания (m_i64LineSizeEl >= m_i64W)

	int8_t	*m_pData;			// Не выровненная отведенная под данные память
	uint16_t	**m_ppRows;		// Массив выровненых строк, указатель на первую строку - начало выровненных данных, все строки идут друг за другом, количество элементов в строке - m_i64LineSizeEl
};
//================================================================
//================================================================

//================================================================
// class MFData
//================================================================
class MFData
{
public:
	MFData();
	~MFData();
	void deleteData();

	int iRead(const char *a_pcFileName);
	int iCreate(int64_t a_i64W, int64_t a_i64H, int64_t a_i64Pow);
	int iWrite(const char *a_pcFileName);

	inline float *pfGetRow(int64_t a_i64Row) { return m_ppRows[a_i64Row]; }

public:
	int64_t	m_i64W;			// Ширина
	int64_t	m_i64H;			// Высота
	int64_t	m_i64Pow;			// Выравнивание (степень 2)
	int64_t	m_i64LineSizeEl;		// Длина строки в элементах, с учетом выравнивания (m_i64LineSizeEl >= m_i64W)

	int8_t	*m_pData;			// Не выровненная отведенная под данные память
	float	**m_ppRows;		// Массив выровненых строк, указатель на первую строку - начало выровненных данных, все строки идут друг за другом, количество элементов в строке - m_i64LineSizeEl
};
//================================================================
//================================================================

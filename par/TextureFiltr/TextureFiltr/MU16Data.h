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
	int64_t	m_i64W;				// ������
	int64_t	m_i64H;				// ������
	int64_t	m_i64Pow;			// ������������ (������� 2)
	int64_t	m_i64LineSizeEl;		// ����� ������ � ���������, � ������ ������������ (m_i64LineSizeEl >= m_i64W)

	int8_t	*m_pData;			// �� ����������� ���������� ��� ������ ������
	uint16_t	**m_ppRows;		// ������ ���������� �����, ��������� �� ������ ������ - ������ ����������� ������, ��� ������ ���� ���� �� ������, ���������� ��������� � ������ - m_i64LineSizeEl
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
	int64_t	m_i64W;			// ������
	int64_t	m_i64H;			// ������
	int64_t	m_i64Pow;			// ������������ (������� 2)
	int64_t	m_i64LineSizeEl;		// ����� ������ � ���������, � ������ ������������ (m_i64LineSizeEl >= m_i64W)

	int8_t	*m_pData;			// �� ����������� ���������� ��� ������ ������
	float	**m_ppRows;		// ������ ���������� �����, ��������� �� ������ ������ - ������ ����������� ������, ��� ������ ���� ���� �� ������, ���������� ��������� � ������ - m_i64LineSizeEl
};
//================================================================
//================================================================

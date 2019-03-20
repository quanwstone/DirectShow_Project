#pragma once
struct  stuData
{
	void *pData;
	stuData *next;
};

template<class T>
class CQueue_
{
public:
	CQueue_() :m_pHead(nullptr), m_iCurrPush(0), m_iCurrPop(0), m_iQueueTotal(0)
	{
		InitializeCriticalSection(&m_cs);
	};
	~CQueue_() {
		DeleteCriticalSection(&m_cs);

		DestoryList();
	};

public:
	void initlist(int iTotal)
	{
		m_iQueueTotal = iTotal;
		
		T *m_pHead = new T[iTotal];
	}
	bool push_back(T *p)
	{
		EnterCriticalSection(&m_cs);

		if (m_iCurrPush > m_iQueueTotal)
		{
			m_iCurrPush = m_iCurrPush % m_iQueueTotal;
		}
		m_pHead[m_iCurrPush] = p;
		m_iCurrPush++;

		LeaveCriticalSection(&m_cs);

		return true;
	}

	T *pop()
	{
		if(m_pHead == nullptr)
			return nullptr;

		EnterCriticalSection(&m_cs);

		if (m_iCurrPop > m_iQueueTotal)
		{
			m_iCurrPop = m_iCurrPop % m_iQueueTotal;
		}

		T *pTemp = m_pHead[m_iCurrPop];
		m_iCurrPop++;
		
		LeaveCriticalSection(&m_cs);

		return pTemp;
	}

	void DestoryList()
	{
		delete[] m_pHead;
		m_pHead = nullptr;
	}
private:

	CRITICAL_SECTION m_cs;

	int m_iCurrPush;
	int m_iCurrPop;
	int m_iQueueTotal;

	T *m_pHead;
};
/***************************************************************************************

----------------------------- Copyright(C) 2010-2020 -----------------------------------

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File name:		LazyLock.h
Author:			LazyBear
Version:		1.0.0
Date:			2014-10-17
Description:	this file define utility micros for programming
Notes:        

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Modify History:
 Date					Author					Modification
 2014-10-17				  zwu					   Create
-----------------------------------------------------------------------------------------
*****************************************************************************************/
#ifndef LAZY_UTILITY_LOCK_H_
#define LAZY_UTILITY_LOCK_H_

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#define CRITICAL_SECTION						pthread_mutex_t
#define InitializeCriticalSection(px)			pthread_mutex_init(px, 0)
#define DeleteCriticalSection(px)				pthread_mutex_destroy(px)
#define EnterCriticalSection(px)				pthread_mutex_lock(px)
#define LeaveCriticalSection(px)				pthread_mutex_unlock(px)
#endif

namespace LazyUtility
{
class CLazyCriSec
{
public:
	//constructor
	CLazyCriSec()
	{
		//create critical section
		m_pCriSec = new CRITICAL_SECTION;

		if(m_pCriSec)
		{
			//init critical section
			InitializeCriticalSection(m_pCriSec);
		}
	}

	//destructor
	~CLazyCriSec()
	{
		if(m_pCriSec)
		{
			//destroy critical section
			DeleteCriticalSection(m_pCriSec);

			//free critical section
			delete m_pCriSec;

			m_pCriSec = NULL;
		}
	}

	//try to lock critical section, succeeded return true else return false
	bool TryLock()
	{
		if(m_pCriSec == NULL)
		{
			return false;
		}
		//try to lock
#ifdef WIN32
		//windows, succeeded return TRUE,failed return FALSE
		if(!TryEnterCriticalSection(m_pCriSec))
#else
		//pthread mutex, succeeded return 0, else failed
		if(pthread_mutex_trylock(m_pCriSec))
#endif
		{
			//fail, return false
			return false;
		}

		//succeeded, the critical section has been lock
		return true;
	}

	//lock critical section, it will be block when it lock by other threads
	void Lock()
	{
		EnterCriticalSection(m_pCriSec);
		//lazylog(LAZY_LOG_LEVEL_MAIN_INFO|OTHER_INFO, "CLazyCriSec::Lock after EnterCriticalSection m_pCriSec:%p\n", m_pCriSec);
	}

	//unlock critical section
	void UnLock()
	{
		LeaveCriticalSection(m_pCriSec);
	}
	
protected:
	CRITICAL_SECTION*			m_pCriSec;//critical section ptr
};

class CLazyLock
{
public:
	//constructor
	CLazyLock(CLazyCriSec* plock):m_pLock(plock)
	{
		//lock critical section if exist
		if(m_pLock)
		{
			m_pLock->Lock();
		}
	}

	//destructor
	~CLazyLock(void)
	{
		//unlock critical section if exist
		if(m_pLock)
		{
			m_pLock->UnLock();
			m_pLock = NULL;
		}
	}

protected:
	CLazyCriSec*		m_pLock;
};
}
#endif

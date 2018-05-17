#ifndef __THREAD_PACKET_H__ 
#define __THREAD_PACKET_H__ 
typedef unsigned int    myThreadParm;//32
//typedef unsigned long   myThreadParm;//64

#ifdef WIN32	//define the macro for Win32 thread 
#include <windows.h> 
#include <process.h>   
 
/*************  mutex (use CRITICAL_SECTION in windows) ***************/ 
#define		THREAD_MUTEX				CRITICAL_SECTION 
#define		INITIALIZE_MUTEX(mutex)		InitializeCriticalSection(mutex) 
#define		DESTROY_MUTEX				DeleteCriticalSection 
#define		LOCK_MUTEX					EnterCriticalSection 
#define		UNLOCK_MUTEX				LeaveCriticalSection 
#define		THREAD_HANDLE				unsigned long 
/**********************************************************************/ 
 
typedef myThreadParm (__stdcall * THREAD_FUN_TYPE)(void *); 
 
/*********************************************************************** 
**  Function : This function is used to create a new thread. The new thread's 
               function has the form as follows:  
			         long (* pThreadFun) (void *)   
			   (one argument which is void * and return an integer type data) 
                  
**      Input: pThreadFun -- the address of function to create a new thread 
**                 pParam -- the parament of the new thread 
 
**  return the handle of the new thread 
************************************************************************/ 
inline THREAD_HANDLE CREATE_THREAD(myThreadParm (* pThreadFun)( void * ),  void * pParam) 
{ 
	return _beginthreadex(NULL, 0, (THREAD_FUN_TYPE) pThreadFun, pParam, NULL, NULL); 
} 
 
/*********************************************************************** 
**  Function : This function is used to end a thread. This function is  
               usually used at the end of the Thread function.  
************************************************************************/ 
inline void END_THREAD() 
{ 
	_endthreadex(0); 
} 
 
/*********************************************************************** 
**  Function : This function is used to close the handle of a thread. 
**     input : hThread -- the handle of the thread. 
************************************************************************/ 
inline void CLOSE_THREAD(THREAD_HANDLE hThread) 
{ 
	CloseHandle((HANDLE *) hThread); 
} 
 
/*********************************************************************** 
**  Function : This function is used to wait for another thread. 
               So the main thread used this function will suspend 
			   until the waiting thread finished. 
 
**     input : hThread -- the handle of waiting thread. 
************************************************************************/ 
inline void WAIT_THREAD(THREAD_HANDLE hThread) 
{ 
	WaitForSingleObject((HANDLE*)hThread, INFINITE); 
} 
 
#else	//define the macro for POSIX thread 
 
#include <pthread.h> 
 
/*************  mutex (use CRITICAL_SECTION in windows) ***************/ 
#define		THREAD_MUTEX				pthread_mutex_t 
#define		INITIALIZE_MUTEX(mutex)		pthread_mutex_init(mutex, NULL)		 
#define		DESTROY_MUTEX				pthread_mutex_destroy 
#define    	LOCK_MUTEX					pthread_mutex_lock 
#define		UNLOCK_MUTEX				pthread_mutex_unlock 
#define		THREAD_HANDLE				pthread_t 
/**********************************************************************/ 
 
 
typedef void *( * THREAD_FUN_TYPE)(void *); 
 
/*********************************************************************** 
**  Function : This function is used to create a new thread. The new thread's 
               function has the form as follows:  
			         int (* pThreadFun) (void *)   
			   (one argument which is void * and return an integer type data) 
                  
**      Input: pThreadFun -- the address of function to create a new thread 
**                 pParam -- the parament of the new thread 
 
**  return the handle of the new thread 
************************************************************************/ 
inline THREAD_HANDLE CREATE_THREAD(int (* pThreadFun)( void * ),  void * pParam) 
{ 
	pthread_t threadID;  
	pthread_create(&threadID, NULL, (THREAD_FUN_TYPE)pThreadFun, pParam);  
	return threadID; 
} 
 
/*********************************************************************** 
**  Function : This function is used to end a thread. This function is  
               usually used at the end of the Thread function.  
************************************************************************/ 
inline void END_THREAD() 
{ 
	pthread_exit(0); 
} 
 
/*********************************************************************** 
**  Function : This function is used to close the handle of a thread. 
	           But the POSIX thread use the pthread_join function to 
			   clean the handle of the thread. So this funciton do 
			   nothing in Linux platform. 
************************************************************************/ 
inline void CLOSE_THREAD(THREAD_HANDLE hThread) 
{ 
	 
} 
 
/*********************************************************************** 
**  Function : This function is used to wait for another thread. 
               So the main thread used this function will suspend 
			   until the waiting thread finished. 
 
**     input : hThread -- the handle of waiting thread. 
************************************************************************/ 
inline void WAIT_THREAD(THREAD_HANDLE hThread) 
{ 
	void * retval; 
	pthread_join(hThread, &retval); 
} 
 
#endif  //WIN32 
 
#endif //__THREAD_PACKET_H__ 
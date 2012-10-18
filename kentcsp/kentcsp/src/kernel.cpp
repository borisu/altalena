/*
*	The Kent C++CSP Library 
*	Copyright (C) 2002-2007 Neil Brown
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cppcsp.h"
#include <iostream>
#include "thread_local.h"

#ifdef XPTLS
extern "C" BOOL WINAPI StoreData(DWORD dw);
extern "C" BOOL WINAPI GetData(DWORD *pdw);
#endif



using namespace csp;
using namespace csp::internal;
using namespace std;

std::list<internal::Process const *> Kernel::KernelData::blocks; 
PureSpinMutex Kernel::KernelData::blocksMutex;
Kernel::KernelData* Kernel::KernelData::originalThreadKernelData = NULL;
bool Kernel::KernelData::deadlocked = false;

template <>
__CPPCSP_ALIGNED_USIGN32 _AtomicProcessQueue< MutexAndEvent<OSBlockingMutex,1> >::ThreadsRunning = 0;
template <>
__CPPCSP_ALIGNED_USIGN32 _AtomicProcessQueue< MutexAndEvent<PureSpinMutex,1> >::ThreadsRunning = 0;
template <>
__CPPCSP_ALIGNED_USIGN32 _AtomicProcessQueue< Condition<1> >::ThreadsRunning = 0;

namespace
{
#ifndef XPTLS
	DeclareThreadLocalPointer(Kernel) __gkernel;
#endif
}

void Kernel::ThreadFunc(void* value)
{
	Kernel* kernel = static_cast<Kernel*>(value);
#ifdef XPTLS
	StoreData((DWORD)kernel);
#elif
	__gkernel = kernel;
#endif

	//std::cerr << "INITNEWTHREAD, kernel:" << __gkernel << std::endl;

	ProcessPtr mainProcess = kernel->currentProcess();

	kernel->initNewThread();

	//std::cerr << "DONE-INITNEWTHREAD" << std::endl;

	
	try
	{
		mainProcess->runProcess();
		//std::cerr << "PROCESS FINISHED RUNNING" << std::endl;
		mainProcess->endProcess();
		//std::cerr << "PROCESS ENDED" << std::endl;
		
	}
	catch (std::exception& e)
	{
		std::cerr << "Uncaught exception from process: " << e.what() << std::endl;
	}

	kernel->destroyInThread();	
	
	Kernel::DestroyThreadKernel();
	
	//std::cerr << "THREAD FINISHED" /*<< GetCurrentThread()*/ << std::endl;

}

template <>
volatile usign32 _AtomicProcessQueue< MutexAndEvent<OSBlockingMutex,1> >::waitFP_calls = 0;
template <>
volatile usign32 _AtomicProcessQueue< MutexAndEvent<PureSpinMutex,1> >::waitFP_calls = 0;
template <>
volatile usign32 _AtomicProcessQueue< Condition<1> >::waitFP_calls = 0;

namespace csp
{
	ThreadId CurrentThreadId()
	{
#ifdef XPTLS
		DWORD kernel = NULL;
		GetData(&kernel);
		return (ThreadId)kernel;
#elif
		return __gkernel;
#endif

		
	}

	namespace internal
	{
	
		internal::ProcessPtr TestInfo::getProcessPtr(CSProcess* process)
		{
			return static_cast<internal::ProcessPtr>(process);
		}
	
		Kernel* GetKernel()
		{
#ifdef XPTLS
			DWORD kernel = NULL;
			GetData(&kernel);
			return (Kernel*)kernel;
#elif
			return __gkernel;
#endif

		}	
		
		Kernel* Kernel::AllocateThreadKernel()
		{
			Kernel* kernel = new Kernel;

#ifdef XPTLS
			StoreData((DWORD)kernel);
#elif
			__gkernel = kernel;
#endif
			return kernel;
		}
		
		void Kernel::DestroyThreadKernel()
		{
#ifdef XPTLS
			DWORD kernel = NULL;
			GetData(&kernel);
			delete (Kernel*)kernel;
			StoreData((DWORD)NULL);
#elif
			delete __gkernel;
#endif
			
		}
		
		void Kernel::InitNewThread(Kernel* kernel)
		{
			kernel->data.threadId = csp::CurrentThreadId();
			#ifdef CPPCSP_FIBERS
				kernel->data.currentProcess->context = ConvertThreadToFiber(kernel->data.currentProcess);
			#endif
		}
		
		void Kernel::DestroyInThread(Kernel* kernel)
		{
			#ifdef CPPCSP_FIBERS
				ConvertFiberToThread();
			#endif
		
			//delete the initial process:
			delete kernel->data.initialProcess;
		}
		
		void ContextSwitch(Context* from, Context* to);
		
		bool Kernel::ReSchedule(KernelData* data)
		{					
			ProcessPtr oldProcess = data->currentProcess;

			Time t;
			Time* pt = NULL;
			
			try
			{
				do
				{
					pt = NULL;
					data->timeoutQueue.checkTimeouts();
			
					if (data->timeoutQueue.haveTimeouts())
					{
						t = data->timeoutQueue.soonestTimeout();
							pt = &t;
					}

					//Get the next process to run:
					data->currentProcess = data->runQueue.popHead(pt);
				}
				while (data->currentProcess == NullProcessPtr);
					
			}
			catch (DeadlockException)
			{
				std::cerr << "Deadlock!" << std::endl;
				
				KernelData::DumpBlocks(std::cerr);
			
				//switch to initial process and throw a DeadlockError:
				
				KernelData::deadlocked = true;
				AddProcess(KernelData::originalThreadKernelData,KernelData::originalThreadKernelData->initialProcess,KernelData::originalThreadKernelData->initialProcess);
				
				//We can't really do anything more, so block:
				data->runQueue.popHead(NULL);
			}
		
			
			//Switch to the new process:
			if (oldProcess != data->currentProcess)
			{
				ContextSwitch(oldProcess == NULL ? NULL : &(oldProcess->context),&(data->currentProcess->context));
				
				//oldProcess is always the process of this currently running function:
				
				if (data == KernelData::originalThreadKernelData && oldProcess == data->initialProcess)
				{
					//We are the very original process -- we might have been woken because everything has deadlocked
					if (KernelData::deadlocked)
					{
						//No need to claim the mutex -- we must be the only process running!
						throw DeadlockError(KernelData::blocks);
					}
				}
			}
			
			//Delete any stacks from old processes:
			for (list< pair<ProcessPtr, ProcessDelInfo> >::iterator it = data->stacksToDelete.begin();it != data->stacksToDelete.end();)
			{
				if (it->first == data->currentProcess)
				{
					it++;
				}
				else
				{
				#ifdef CPPCSP_LONGJMP
					delete [](it->second);
				#endif
				#ifdef CPPCSP_FIBERS
					DeleteFiber(it->second);
				#endif
					#ifdef CPPCSP_MSVC
						it = data->stacksToDelete.erase(it);
					#else
						data->stacksToDelete.erase(it++);
					#endif
				}
			}
			
			return true;
		}
		
		bool Kernel::AddProcess(KernelData* data,internal::ProcessPtr head,internal::ProcessPtr tail)
		{
			if (head == tail)
			{
				//This should be quicker than pushing a chain:
				data->runQueue.pushProcess(head);
			}
			else
			{
				data->runQueue.pushChain(head,tail);
			}
			return true;
		}
	}
}

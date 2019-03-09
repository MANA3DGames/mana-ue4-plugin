#pragma once

#include "MANA3DRuntime/MANA3DAgent.h"
#include "MANA3DRuntime/MANA3DJob.h"

#include <string>
using namespace std;


class FMANA3DThread : public FRunnable
{	
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FMANA3DThread* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;
 
	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;
 

	AMANA3DAgent* agent;
	TArray<AActor*> actors;
	MANA3DJob job;
 
private:
	unsigned static int ThreadCount;

public:

	FMANA3DThread( AMANA3DAgent* agent, TArray<AActor*>& actors, MANA3DJob& job );
	virtual ~FMANA3DThread();
 
	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface
 
	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();
 

	void WriteModelArray( TArray<AMANA3DModel*>& modelArray, TSharedRef<TJsonWriter<>>& Writer );
	std::string CreateZipFile( std::string jsonPath );

 
	/* 
		Start the thread and the worker from static (easy access)! 
		This code ensures only 1 Prime Number thread will be able to run at a time. 
		This function returns a handle to the newly started instance.
	*/
	static FMANA3DThread* Create( AMANA3DAgent* agent, TArray<AActor*>& actors, MANA3DJob& job );
 
	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

};
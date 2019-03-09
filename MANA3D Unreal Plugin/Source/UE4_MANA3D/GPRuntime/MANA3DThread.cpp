#include "UE4_MANA3D.h"
#include "MANA3DThread.h"

#include "StringUtil.h"
#include "MANA3DCurlRequest.h"
#include "UEUtil.h"
#include "ZlibUtil.h"


//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FMANA3DThread* FMANA3DThread::Runnable = NULL;
//***********************************************************
unsigned int FMANA3DThread::ThreadCount = 0;


FMANA3DThread::FMANA3DThread( AMANA3DAgent* aAgent, TArray<AActor*>& aActors, MANA3DJob& MANA3DJob )
	: agent(NULL)
{
	agent = aAgent;
	actors = aActors;
	job = MANA3DJob;

	Thread = FRunnableThread::Create( this, *( FString::Printf( TEXT( "FMANA3DThread%i" ), ThreadCount ) ), 0, TPri_BelowNormal );
	ThreadCount++;
}
 
FMANA3DThread::~FMANA3DThread()
{
	delete Thread;
	Thread = NULL;
}
 

bool FMANA3DThread::Init()
{
	//Init the Data 
	return true;
}
 
uint32 FMANA3DThread::Run()
{
	//Initial wait before starting
	//FPlatformProcess::Sleep( 0.03 );

	if ( agent == NULL ) return 0;

	FString fImMANA3Dath = FString::Printf( TEXT( "%sCapture.png" ), *AMANA3DAgent::GetTempFilesPath() );
	FScreenshotRequest::RequestScreenshot( fImMANA3Dath, false, false );
	std::string imMANA3Dath = StringUtil::TCHAR_to_charPointer( *fImMANA3Dath ); 

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create( &OutputString );
		
	Writer->WriteObjectStart();
		
		Writer->WriteArrayStart( "models" );
			WriteModelArray( job.data->models, Writer );
		Writer->WriteArrayEnd();
		
		Writer->WriteArrayStart( "additionalModels" );
			WriteModelArray( job.data->additionalModels, Writer );
		Writer->WriteArrayEnd();

		Writer->WriteValue( "bounds", "" );
		Writer->WriteValue( "material", "" );
		Writer->WriteValue( "flip", job.data->flip );

	Writer->WriteObjectEnd();

	Writer->Close();

	FString captureFile = FString::Printf( TEXT( "%sCapture.json" ), *AMANA3DAgent::GetTempFilesPath() );
	FFileHelper::SaveStringToFile( OutputString, *captureFile );

	const char* captureZipFilePath = StringUtil::TCHAR_to_charPointer( *captureFile );
	std::string zipFilePath = CreateZipFile( captureZipFilePath );

	const TCHAR* appID_TCHAR = *agent->AppID.ToString();
	const TCHAR* secretKey_TCHAR = *agent->SecretKey.ToString();
	const char* appidptr = StringUtil::TCHAR_to_charPointer( appID_TCHAR );
	const char* secretptr = StringUtil::TCHAR_to_charPointer( secretKey_TCHAR );
	const char* nameptr = StringUtil::TCHAR_to_charPointer( *agent->cName );
	const char* emailptr = StringUtil::TCHAR_to_charPointer( *agent->cEmail );
	MANA3DCurlRequest::SubmitCapture( appidptr, secretptr, "0", nameptr, emailptr, zipFilePath.c_str(), imMANA3Dath.c_str() );

	agent->OnComplete();

	//Run FMANA3DThread::Shutdown() from the timer in Game Thread that is watching
    //to see when v::IsThreadFinished()
 
	return 0;
}
 
void FMANA3DThread::Stop()
{
	StopTaskCounter.Increment();
}

 
FMANA3DThread* FMANA3DThread::Create( AMANA3DAgent* agent, TArray<AActor*>& actors, MANA3DJob& job )
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FMANA3DThread( agent, actors, job );			
	}
	return Runnable;
}
 
void FMANA3DThread::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}
 
void FMANA3DThread::Shutdown()
{
	if ( Runnable )
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}
 

void FMANA3DThread::WriteModelArray( TArray<AMANA3DModel*>& modelArray, TSharedRef<TJsonWriter<>>& Writer )
{
	for ( AMANA3DModel* model : modelArray )
	{
		Writer->WriteObjectStart();
			
			Writer->WriteValue( "ObjID", model->ObjID );

			Writer->WriteArrayStart( "matrix" );
			for ( int i = 0; i < 4; i++ )
			{
				Writer->WriteArrayStart();
				for ( int j = 0; j < 4; j++ )
					Writer->WriteValue( FString::SanitizeFloat( model->matrix[i][j] ) );
				Writer->WriteArrayEnd();
			}
			Writer->WriteArrayEnd();

			Writer->WriteValue( "meshID", model->meshID );
			Writer->WriteValue( "materials", model->materials );
			Writer->WriteValue( "meshData", model->meshData );

		Writer->WriteObjectEnd();
	}
}

std::string FMANA3DThread::CreateZipFile( std::string jsonPath )
{
	// append the existing file
	FString zipFStr = FString::Printf( TEXT( "%sConfig.zip" ), *AMANA3DAgent::GetTempFilesPath() );
	const char* zipPath = StringUtil::TCHAR_to_charPointer( *zipFStr );
	
	ZlibUtil::CreateZipFile( zipPath, jsonPath );

	return std::string( zipPath );
}


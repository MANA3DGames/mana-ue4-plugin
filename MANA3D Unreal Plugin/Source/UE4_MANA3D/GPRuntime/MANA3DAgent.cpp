#include "UE4_MANA3D.h"
#include "MANA3DAgent.h"

#include "Utilities/UEUtil.h"
#include "MANA3DThread.h"


AMANA3DAgent::AMANA3DAgent( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	// This may not work at the beginning as the GEngine is still NULL at this point.
	world = UEUtil::GetWorld();
	LoadIDPairsFile();
}

void AMANA3DAgent::Destroyed()
{
	if ( assetIDPairs != NULL )
		delete assetIDPairs;
}

void AMANA3DAgent::LoadIDPairsFile()
{
	FString path = GetAssetIDPairsFilePath();

	if ( FPaths::FileExists( path ) )
	{
		FString JsonRaw;
		FFileHelper::LoadFileToString( JsonRaw, *path );
		TSharedPtr<FJsonObject> JsonParsed = MakeShareable( new FJsonObject );
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create( JsonRaw );

		if ( !FJsonSerializer::Deserialize( JsonReader, JsonParsed ) )
			GLog->Log( "Failed to Deserialize IDPairs JSON file" );
		else
		{
			TArray<TSharedPtr<FJsonValue>> meshArray = JsonParsed->GetArrayField( "MeshIDPairs" );
			TArray<TSharedPtr<FJsonValue>> textureArray = JsonParsed->GetArrayField( "TextureIDPairs" );

			TArray<MANA3DIDPair> meshIDPairs, textureIDPairs;
			
			for ( TSharedPtr<FJsonValue> val : meshArray )
			{
				TSharedPtr<FJsonValue> value = val;
				TSharedPtr<FJsonObject> json = value->AsObject();
				
				TArray<FString, FDefaultAllocator> keys;
				json->Values.GetKeys( keys );

				MANA3DIDPair pair( keys[0], FCString::Atoi( *json->GetStringField( keys[0] ) ) );
				meshIDPairs.Add( pair );
			}

			for ( TSharedPtr<FJsonValue> val : textureArray )
			{
				TSharedPtr<FJsonValue> value = val;
				TSharedPtr<FJsonObject> json = value->AsObject();
				
				TArray<FString, FDefaultAllocator> keys;
				json->Values.GetKeys( keys );

				MANA3DIDPair pair( keys[0], FCString::Atoi( *json->GetStringField( keys[0] ) ) );
				textureIDPairs.Add( pair );
			}

			assetIDPairs = new MANA3DAssetIDPairs();
			assetIDPairs->meshIDPairs = meshIDPairs;
			assetIDPairs->textureIDPairs = textureIDPairs;
		}
	}
}

void AMANA3DAgent::Print( FString sName, FString email )
{
	if ( world == NULL )
		world = GetWorld();

	if ( assetIDPairs == NULL )
		LoadIDPairsFile();

	if ( _isUploading ) return;
	_isUploading = true;

	cName = sName;
	cEmail = email;

	TArray<AActor*> actors;
	UEUtil::GetTaggedActors( world, actors );

	GLog->Log( ELogVerbosity::Display, FString( "[MANA3D_MESH] Actors Count:  " ) + FString::FromInt( actors.Num() ) );

	MANA3DJob job;
	job.data->flip = true;
	for ( AActor* actor : actors )
		job.AddModel( actor, this );
	
	FMANA3DThread::Create( this, actors, job );
}

void AMANA3DAgent::OnComplete()
{
	_isUploading = false;
}


int AMANA3DAgent::GetMeshID( FString sourceID )
{
	if ( assetIDPairs != NULL )
	{
		for ( MANA3DIDPair pair : assetIDPairs->meshIDPairs )
		{
			if ( pair.sourceID == sourceID )
				return pair.id;
		}
	}
	else
		GLog->Log( ELogVerbosity::Warning, "AssetIDPairs hasn't been initialized yet...." );
	
    return -1;
}

int AMANA3DAgent::GetTextureID( FString sourceID )
{
	if ( assetIDPairs != NULL )
	{
		for ( MANA3DIDPair pair : assetIDPairs->textureIDPairs )
		{
			if ( pair.sourceID == sourceID )
				return pair.id;
		}
	}
	else
		GLog->Log( ELogVerbosity::Warning, "AssetIDPairs hasn't been initialized yet...." );

    return -1;
}



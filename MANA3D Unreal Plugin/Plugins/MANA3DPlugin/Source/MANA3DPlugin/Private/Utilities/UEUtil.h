#pragma once

class UEUtil
{

public:

	static UWorld* GetWorld()
	{
		if ( !GEngine )
		{
			GLog->Log( ELogVerbosity::Warning, "GEngine is NULL!" );
			return NULL;
		}

		UWorld* world = NULL;
		const TIndirectArray<FWorldContext>& contexts = GEngine->GetWorldContexts();
		for ( int32 i = 0; i < contexts.Num(); i++ )
		{
			if ( contexts[i].WorldType == EWorldType::PIE )
				return contexts[i].World();

			if ( contexts[i].WorldType == EWorldType::Editor )
				world = contexts[i].World();
		}

		return world;
	}

	static void GetTaggedActors( UWorld* inWorld, TArray<AActor*>& actors )
	{
		if ( inWorld == NULL ) 
		{
			GLog->Log( ELogVerbosity::Error, "inWorld is NULL!" );
			return;
		}

		// Find Actors by tag (also works on ActorComponents, use TObjectIterator instead)
		for ( TActorIterator<AActor> It( inWorld ); It; ++It )
		{
			AActor* actor = *It;
			if ( actor->ActorHasTag( FName( TEXT( "MANA3D_MESH" ) ) ) )
				actors.Add( actor );
		}
	}

};


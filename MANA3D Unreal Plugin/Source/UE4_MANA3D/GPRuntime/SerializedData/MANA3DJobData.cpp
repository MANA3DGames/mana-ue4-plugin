#include "UE4_MANA3D.h"
#include "MANA3DJobData.h"


AMANA3DJobData::AMANA3DJobData( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
}

AMANA3DJobData::~AMANA3DJobData()
{
	while ( models.Num() > 0 )
	{
		if ( models[0] != NULL )
		{
			AMANA3DModel* temp = models[0];
			models.RemoveAt( 0 );
			temp->Destroy();
		}
	}

	while ( additionalModels.Num() > 0 )
	{
		if ( additionalModels[0] != NULL )
		{
			AMANA3DModel* temp = additionalModels[0];
			additionalModels.RemoveAt( 0 );
			temp->Destroy();
		}
	}
}
#pragma once

#include "GameFramework/Actor.h"
#include "MANA3DMeshIDPair.h"
#include "MANA3DModel.generated.h"

UCLASS(MinimalAPI)
class AMANA3DModel : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	FString ObjID;
	FString meshID;
	FString materials;
	FString meshData;
	float matrix[4][4];

	int animatedModelID;
};


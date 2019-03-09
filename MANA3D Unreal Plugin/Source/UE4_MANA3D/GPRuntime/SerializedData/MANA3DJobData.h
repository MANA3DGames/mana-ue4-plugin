#pragma once

#include "GameFramework/Actor.h"
#include "MANA3DModel.h"
#include "MANA3DJobData.generated.h"

UCLASS(MinimalAPI)
class AMANA3DJobData : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	TArray<AMANA3DModel*> models;
	TArray<AMANA3DModel*> additionalModels;
	TArray<float> bounds;
	FString material;
	bool flip;

	~AMANA3DJobData();
};
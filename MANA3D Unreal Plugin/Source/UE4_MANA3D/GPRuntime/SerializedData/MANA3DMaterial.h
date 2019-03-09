#pragma once

#include "MANA3DMaterial.generated.h"

USTRUCT()
struct FMANA3DMaterial
{
	GENERATED_BODY()

	FString name;
	FString textureName;
	FLinearColor color;
	int textureID;
};
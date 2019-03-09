#pragma once

#include "MANA3DAgent.h"
#include "SerializedData/MANA3DJobData.h"
#include "SerializedData/MANA3DMaterial.h"
#include "SerializedData/MANA3DModel.h"

class MANA3DJob
{
public:
	MANA3DJob();
	~MANA3DJob();

	AMANA3DJobData* data;

	void AddModel( AActor* actor, AMANA3DAgent* agent );

	FString GetMaterials( TArray<UMaterialInterface*> mats, AMANA3DAgent* agent );
	FString GetMaterialString( FString name, FMANA3DMaterial material );
	FString MeshToString( FSkeletalMeshVertexBuffer& mesh, 
						  TArray<FVector>& vertices, TArray<uint32>& indices, FString name, 
						  bool invert, bool includeMaterials, int customMaterialID );


	bool GetAnimatedVertexPositions( USkeletalMeshComponent* Mesh,
									 TArray<FVector>& vertices,
									 bool PerformPawnVelocityCorrection );
};

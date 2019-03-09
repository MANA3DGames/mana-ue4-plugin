#pragma once

#include "MANA3DAssetIDPairs.h"
#include "GameFramework/Actor.h"
#include "MANA3DAgent.generated.h"


UCLASS(BlueprintType, Blueprintable, config = Engine, meta = (ShortTooltip = "MANA3D Agent."))
class AMANA3DAgent : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	bool _isUploading;

	bool FindMANA3DManagerInstance();

	

public:
	FString cName, cEmail;

	UWorld* world;

	MANA3DAssetIDPairs* assetIDPairs;

	void LoadIDPairsFile();

	UFUNCTION( BlueprintCallable, Category = "MANA3D" )
	void Print( FString sName, FString email );

	void OnComplete();

	virtual void Destroyed() override;



	UPROPERTY(EditAnywhere)
	FText AppID;
	UPROPERTY(EditAnywhere)
	FText SecretKey;

	int GetMeshID( FString sourceID );
	int GetTextureID( FString sourceID );


private:
	static FString GetRootPath()
	{
		FString GameRootDirPath;
#if WITH_EDITOR
		GameRootDirPath = FPaths::GameSourceDir();
		GameRootDirPath.RemoveFromEnd( "Source/", ESearchCase::IgnoreCase );
#else
		GameRootDirPath = FPaths::GameDir();
#endif
		return GameRootDirPath;
	}

	static FString GetMANA3DDataPath()
	{
		FString GameSourceDirPath = GetRootPath();
		GameSourceDirPath = FPaths::Combine( *GameSourceDirPath, TEXT( "MANA3DData/" ) );
		return GameSourceDirPath;
	}

	static FString GetAssetIDPairsFilePath()
	{
		return FPaths::Combine( *GetMANA3DDataPath(), TEXT( "AssetIDPairs.json" ) );
	}

public:
	static FString GetTempFilesPath()
	{
		return FPaths::Combine( *GetMANA3DDataPath(), TEXT( "Temp/" ) );
	}

};


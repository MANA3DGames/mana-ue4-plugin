#pragma once

#include <string>
using namespace std;

#include "MANA3DAssetIDPairs.h"

#include "GameFramework/Actor.h"
#include "MANA3DManager.generated.h"


UENUM(BlueprintType)
enum class EBAKEASSETFILTER : uint8
{
	MANA3DE_ASSETSARRAY_TAGGED 	UMETA( DisplayName = "Asset Arrays And Tagged Actor[MANA3D_MESH]" ),
	MANA3DE_ASSETSARRAY 		UMETA( DisplayName = "Asset Arrays Only" ),
	MANA3DE_TAGGED				UMETA( DisplayName = "Tagged Actor[MANA3D_MESH] Only" )
};

UCLASS(BlueprintType, Blueprintable, config = Engine, meta = (ShortTooltip = "MANA3D Manager."))
class AMANA3DManager : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere)
	FText AppID;
	UPROPERTY(EditAnywhere)
	FText SecretKey;

	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> Meshes;
	UPROPERTY(EditAnywhere)
	TArray<UTexture2D*> Textures;

	UPROPERTY(EditAnywhere)
	EBAKEASSETFILTER BakeFilter;


	MANA3DAssetIDPairs* assetIDPairs;


	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;
	virtual void Destroyed() override;

	// Bake all the assets that associated with MANA3DManger instance in the scene.
	void Bake( UWorld* world );

	void GetStaticMeshesAndTextures( TArray<AActor*>& actors, TArray<std::string>& objPaths, TArray<std::string>& texturePaths );

	std::string ExportPNG( UTexture* texture );
	std::string ExportOBJ( UStaticMesh* mesh, AActor* actor );
	std::string CreateZipFile( TArray<std::string>& objPaths, TArray<std::string>& texturePaths );
	
	void SubmitAssets( const TCHAR* appID, const TCHAR* secretKey, const char* zipPath );
	
	void SerializeAssetsData();


	int AddMesh( UStaticMesh* mesh );
	int AddTexture( UTexture2D* texture );

	int GetMeshID( FString sourceID );
	int GetTextureID( FString sourceID );


	void ClearIDPairs();
	void LoadIDPairsFile();



	static FString GetRootPath()
	{
		FString GameRootDirPath = FPaths::GameSourceDir();
		GameRootDirPath.RemoveFromEnd( "Source/", ESearchCase::IgnoreCase );
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

	static FString GetTempFilesPath()
	{
		return FPaths::Combine( *GetMANA3DDataPath(), TEXT( "Temp/" ) );
	}

	static FString GetPluginsPath()
	{
		return FPaths::Combine( *GetRootPath(), TEXT( "Plugins/" ) );
	}

	static FString GetResourcesPath()
	{
		return FPaths::Combine( *GetRootPath(), TEXT( "Plugins/MANA3DPlugin/Resources/" ) );
	}
};
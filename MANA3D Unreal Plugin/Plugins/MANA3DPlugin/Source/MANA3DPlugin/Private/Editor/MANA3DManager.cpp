#include "MANA3DPluginPch.h"
#include "MANA3DManager.h"

#include "Utilities/StringUtil.h"
#include "Utilities/MANA3DCurlRequest.h"
#include "Utilities/UEUtil.h"

#include "zipper.h"
using namespace::ziputils;

#include <fstream>
using namespace::std;



AMANA3DManager::AMANA3DManager( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	LoadIDPairsFile();
}

void AMANA3DManager::Destroyed()
{
	if ( assetIDPairs != NULL )
		delete assetIDPairs;
}


void AMANA3DManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AMANA3DManager::BeginPlay()
{
	Super::BeginPlay();
}


void AMANA3DManager::ClearIDPairs()
{
	if ( assetIDPairs == NULL ) 
	{
		GLog->Log( ELogVerbosity::Warning, "AssetIDPairs hasn't been initialized yet!" );
		return;
	}

	assetIDPairs->meshIDPairs.Empty();
	assetIDPairs->textureIDPairs.Empty();
}

void AMANA3DManager::LoadIDPairsFile()
{
	FString path = GetAssetIDPairsFilePath();

	if ( FPaths::FileExists( path ) )
	{
		FString JsonRaw;
		FFileHelper::LoadFileToString( JsonRaw, *path );
		TSharedPtr<FJsonObject> JsonParsed = MakeShareable( new FJsonObject );
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create( JsonRaw );

		if ( !FJsonSerializer::Deserialize( JsonReader, JsonParsed ) )
			GLog->Log( ELogVerbosity::Error, "Failed to Deserialize IDPairs JSON file" );
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


void AMANA3DManager::Bake( UWorld* world )
{
	// Clear all saved IDPairs.
	if ( assetIDPairs != NULL )
		ClearIDPairs();
	else
		assetIDPairs = new MANA3DAssetIDPairs();

	TArray<UStaticMesh*> outMeshes;
	TArray<UTexture*> outTextures;

	// Lists to hold the paths for all exported assets (obj and png files).
	TArray<std::string> objPaths, texturePaths;

	if ( BakeFilter == EBAKEASSETFILTER::MANA3DE_ASSETSARRAY_TAGGED || 
		 BakeFilter == EBAKEASSETFILTER::MANA3DE_TAGGED )
	{
		TArray<AActor*> actors;
		UEUtil::GetTaggedActors( world, actors );

		if ( actors.Num() > 0 )
			GetStaticMeshesAndTextures( actors, objPaths, texturePaths );
	}


	if ( BakeFilter == EBAKEASSETFILTER::MANA3DE_ASSETSARRAY_TAGGED ||
		 BakeFilter == EBAKEASSETFILTER::MANA3DE_ASSETSARRAY )
	{
		// Export obj file from the static mesh and add it's path to objPaths array
		for ( UStaticMesh* mesh : Meshes )
			objPaths.Add( ExportOBJ( mesh, NULL ) );

		// Export all the textures as png files and add thier paths to texturePaths list.
		for ( UTexture* tex : Textures )
		{
			if ( !tex->IsNormalMap() )
				texturePaths.Add( ExportPNG( tex ) );
		}
	}

	// Create assets file (zipfile) which will be submitted to the server.
	std::string zipPath = CreateZipFile( objPaths, texturePaths );

	// Convert AppID and SecretKey to TCHAR* so we can use it later.
	const TCHAR* appID_TCHAR = *AppID.ToString();
	const TCHAR* secretKey_TCHAR = *SecretKey.ToString();


	// Upload the assets file to MANA3D server.
	SubmitAssets( appID_TCHAR, secretKey_TCHAR, zipPath.c_str() );

	// Serialize baked data.
	SerializeAssetsData();
}

void AMANA3DManager::GetStaticMeshesAndTextures( TArray<AActor*>& actors, TArray<std::string>& objPaths, TArray<std::string>& texturePaths )
{
	// Generate and export obj and png files.
	for ( AActor* actor : actors )
	{
		// Get all UStaticMesh components associated with this actor.
		TArray<UStaticMeshComponent*> staticMeshComponents;
		actor->GetComponents<UStaticMeshComponent>( staticMeshComponents );
		for ( UStaticMeshComponent* smComponent : staticMeshComponents )
		{
			// Export obj file from the static mesh and add it's path to objPaths array
			objPaths.Add( ExportOBJ( smComponent->StaticMesh, actor ) );

			// Get all the textures that been used with this static mesh.
			TArray<UTexture*> textures;
			smComponent->GetUsedTextures( textures, EMaterialQualityLevel::High );
			
			// Export all the textures as png files and add thier paths to texturePaths list.
			for ( UTexture* tex : textures )
			{
				if ( !tex->IsNormalMap() )
					texturePaths.Add( ExportPNG( tex ) );
			}
		}

		TArray<USkeletalMeshComponent*> skeletalMeshComponents;
		actor->GetComponents<USkeletalMeshComponent>( skeletalMeshComponents );
		for ( USkeletalMeshComponent* smComponent : skeletalMeshComponents )
		{
			// Get all the textures that been used with this static mesh.
			TArray<UTexture*> textures;
			smComponent->GetUsedTextures( textures, EMaterialQualityLevel::High );
			
			// Export all the textures as png files and add thier paths to texturePaths list.
			for ( UTexture* tex : textures )
			{
				if ( !tex->IsNormalMap() )
					texturePaths.Add( ExportPNG( tex ) );
			}
		}
	}
}


std::string AMANA3DManager::ExportPNG( UTexture* texture )
{
	UTexture2D* tex2d = static_cast<UTexture2D*>( texture );

	if ( tex2d == NULL )
	{
		GLog->Log( ELogVerbosity::Error, "Couldn't cast texture to UTexture2D!!" );
		return std::string();
	}

	int id = AddTexture( tex2d );
	
	// Recommended settigns:
	//	Level of Details -> Mip Gen Settings	 : NoMipmaps
	TextureMipGenSettings originalMipSettings = tex2d->MipGenSettings;
	int32 originalCompressionNone = tex2d->CompressionNone;

	if ( tex2d->MipGenSettings != TextureMipGenSettings::TMGS_NoMipmaps )
	{
#if WITH_EDITORONLY_DATA
		tex2d->CompressionNone = true;
		tex2d->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		tex2d->UpdateResourceW();
#endif
	}

	//	Compression	-> Compression Settigns : VectorDisplacment [PF_B8G8R8A8]
	TextureCompressionSettings originalCompressionSettings = tex2d->CompressionSettings;;

	if ( tex2d->CompressionSettings != TextureCompressionSettings::TC_VectorDisplacementmap )
	{
#if WITH_EDITORONLY_DATA
		tex2d->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		tex2d->UpdateResourceW();
#endif
	}


	// First we need to access the mipmap 0 of the texture, it is important to only access the data we need through refrences or , 
	// like I did here, pointers, either will do.We do this to directly access the original data in the memory instead of copying 
	// that data into a local variable.This saves memory and speed:
	FTexture2DMipMap* mip0 = &tex2d->PlatformData->Mips[0];
	
	// With this, we can access the BulkData which contains the raw image data:
	FByteBulkData* RawImageData = &mip0->BulkData;
	
	// Considering this texture data is used in the rendering thread, we need to lock it to make sure nothing else can access it 
	// while we are using it, if we didn't, we could cause memory corruptions or crashes. So now we lock the image data which will 
	// also return a pointer to it - which we can then go ahead and cast the unformated data into an array of FColor's:
	FColor* FormatedImageData = static_cast<FColor*>( RawImageData->Lock(LOCK_READ_ONLY) );
	if ( FormatedImageData == NULL )
	{
		RawImageData->Unlock();
		GLog->Log( ELogVerbosity::Error, FString::Printf( TEXT( "Couldn't get formated data : %s" ), *tex2d->GetPathName() ) );
		return std::string();
	}

	// The FColor array is ofc 1 dimensional. So to access certain parts of the image, we need to use the width and height of 
	// the texture to calculate the position of the pixel we are wanting to lookup. Here's a small statement that does just that:
	uint8 PixelX = 5, PixelY = 10;
	uint32 TextureWidth = mip0->SizeX, TextureHeight = mip0->SizeY;

	TArray<FColor> colors;
	int32 sizeOfBuffer = TextureWidth * TextureHeight;
	colors.AddUninitialized( sizeOfBuffer );

	for ( int32 i = 0; i < sizeOfBuffer; i++ )
		colors[i] = FormatedImageData[i];

	FIntPoint destSize( TextureWidth, TextureHeight );
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray( destSize.X, destSize.Y, colors, CompressedBitmap );

	FString pnMANA3Dath = FString::Printf( TEXT( "%s%i.png" ), *GetTempFilesPath(), id );
	FFileHelper::SaveArrayToFile( CompressedBitmap, *( pnMANA3Dath ) );

	// And now that we are done with everything, we need to unlock the memory again:
	RawImageData->Unlock();


	// Set back original settings
	if ( tex2d->MipGenSettings != originalMipSettings )
	{
#if WITH_EDITORONLY_DATA
		tex2d->CompressionNone = originalCompressionNone;
		tex2d->MipGenSettings = originalMipSettings;
		tex2d->UpdateResourceW();
#endif
	}

	if ( tex2d->CompressionSettings != originalCompressionSettings )
	{
#if WITH_EDITORONLY_DATA
		tex2d->CompressionSettings = originalCompressionSettings;
		tex2d->UpdateResourceW();
#endif
	}

	return StringUtil::TCHAR_to_charPointer( *pnMANA3Dath );
}

std::string AMANA3DManager::ExportOBJ( UStaticMesh* mesh, AActor* actor )
{
	int id = AddMesh( mesh );

	TArray<FVector> vertices = TArray<FVector>();
	FStaticMeshVertexBuffer *vertexBuffer = &mesh->RenderData->LODResources[0].VertexBuffer;
	FPositionVertexBuffer *positionVertexBuffer = &mesh->RenderData->LODResources[0].PositionVertexBuffer;
	const int32 vertexCount = positionVertexBuffer->GetNumVertices();

	for ( int32 Index = 0; Index < vertexCount; Index++ )
	{
		if ( actor != NULL )
		{
			//This is in the Static Mesh Actor Class, so it is location and tranform of the SMActor
			const FVector WorldSpaceVertexLocation = actor->GetActorLocation() + actor->GetTransform().TransformVector( positionVertexBuffer->VertexPosition( Index ) );
			//add to output FVector array
			vertices.Add( WorldSpaceVertexLocation );
		}
		else
		{
			vertices.Add( positionVertexBuffer->VertexPosition( Index ) );
		}
	}

	// ***************** JSON- String *****************
	FString jsonStr;
	bool invert = true;
	bool includeMaterials = true;
	int32 customMaterialID = -1;

	float factor = invert ? -1 : 1;

	if ( includeMaterials )
	{
		jsonStr.Append("\n");
		jsonStr.Append("mtllib ");
		jsonStr.Append("");
		if ( customMaterialID == -1 )
			// MATERIAL_FILE to be replaced by the material used by the instance not the prefab
			jsonStr.Append( "MATERIAL_FILE.mtl" ).Append("\n");
		else
			jsonStr.Append( FString::Printf( TEXT( "a%i.obj.mtl" ), customMaterialID ) ).Append( "\n" );
	}

	for ( int32 i = 0; i < vertices.Num(); i++ )
	{
		//DrawDebuMANA3Doint( actor->GetWorld(), vertices[i], 10.0f, FColor::Red, false, 1000000 );
		jsonStr.Append( FString::Printf( TEXT( "v %f %f %f\n" ), vertices[i].X, vertices[i].Y, vertices[i].Z + 10 ) );
	}
	jsonStr.Append( "\n" );

	
	// NORMALs
	//foreach( Vector3 lv in mesh.normals )
	//{
	//	sb.Append(string.Format("vn {0} {1} {2}\n", factor * lv.x, lv.y, lv.z));
	//}
	//sb.Append("\n");

	for ( int i = 0; i < vertexCount; i++ )
		jsonStr.Append( FString::Printf( TEXT( "vn %f %f %f\n" ), factor * 1, 0, 0 ) );
	jsonStr.Append( "\n" );



	for (int32 i = 0; i < vertices.Num(); i++)
		jsonStr.Append( FString::Printf( TEXT( "vt %f %f %f\n" ), vertexBuffer->GetVertexUV( i, 0 ).X, vertexBuffer->GetVertexUV( i, 0 ).Y, 0 ) );

	jsonStr.Append("\n");
	jsonStr.Append("g ").Append( "model " + mesh->GetName() ).Append( "\n" );
	jsonStr.Append("\n");


	int32 lodCount = mesh->RenderData->LODResources.Num();
	for (int32 material = 0; material < lodCount; material++)
	{
		jsonStr.Append("usemtl ").Append(FString::Printf(TEXT("M%i"), material)).Append("\n");

		//See if this material is already in the materiallist.
		FRawStaticIndexBuffer* indexBuffer = &mesh->RenderData->LODResources[material].IndexBuffer;

		TArray<int32> triangles;
		for ( int32 i = 0; i < indexBuffer->GetNumIndices(); i++ )
			triangles.Add( indexBuffer->GetArrayView()[i] );

		for ( int i = 0; i < triangles.Num(); i += 3 )
		{
			if ( invert )
			{
				jsonStr.Append( FString::Printf(TEXT("f %i/%i/%i %i/%i/%i %i/%i/%i\n"), 
								triangles[i + 1] + 1, triangles[i + 1] + 1, triangles[i + 1] + 1,
								triangles[i] + 1,	  triangles[i] + 1,		triangles[i] + 1,
								triangles[i + 2] + 1, triangles[i + 2] + 1, triangles[i + 2] + 1));
			}
			else
			{
				jsonStr.Append( FString::Printf(TEXT("f %i/%i/%i %i/%i/%i %i/%i/%i\n"), 
								triangles[i + 2] + 1, triangles[i + 2] + 1, triangles[i + 2] + 1,
								triangles[i] + 1,	  triangles[i] + 1,		triangles[i] + 1,
								triangles[i + 1] + 1, triangles[i + 1] + 1, triangles[i + 1] + 1));
			}
		}
	}

	FString objPath = FString::Printf( TEXT( "%s%i.obj" ), *GetTempFilesPath(), id );

	FBufferArchive ToBinary;
	ToBinary << jsonStr;

	if ( FFileHelper::SaveArrayToFile( ToBinary, *objPath ) )
	{
		// Free Binary Array 	
		ToBinary.FlushCache();
		ToBinary.Empty();
	}
	else
	{
		// Free Binary Array 	
		ToBinary.FlushCache();
		ToBinary.Empty();
	}

	return StringUtil::TCHAR_to_charPointer( *objPath );
}

std::string AMANA3DManager::CreateZipFile( TArray<std::string>& objPaths, TArray<std::string>& texturePaths )
{
	// append the existing file
	FString zipFStr = FString::Printf( TEXT( "%sAssets.zip" ), *GetTempFilesPath() );
	const char* zipPath = StringUtil::TCHAR_to_charPointer( *zipFStr );
	zipper zipFile;
	zipFile.create( zipPath );

	// Add file into a folder
	int count = 0;
	for ( std::string path : objPaths )
	{
		ifstream file( path, ios::in | ios::binary );
		if ( file.is_open() )
		{
			std::string str = "/models/" + std::to_string( count ) + ".obj";
			zipFile.addEntry( str.c_str() );
			zipFile << file;
		}
		file.close();

		if ( std::remove( path.c_str() ) != 0 )
			perror( "Error deleting file" );	// Not working in unreal.

		count++;
	}

	count = 0;
	for ( std::string path : texturePaths )
	{
		ifstream file( path, ios::in | ios::binary );
		if ( file.is_open() )
		{
			std::string str = "/textures/" + std::to_string( count ) + ".png";
			zipFile.addEntry( str.c_str() );
			zipFile << file;
		}
		file.close();

		if ( std::remove( path.c_str() ) != 0 )
			perror( "Error deleting file" );	// Not working in unreal.

		count++;
	}

	zipFile.close();

	return std::string( zipPath );
}


void AMANA3DManager::SubmitAssets( const TCHAR* appID, const TCHAR* secretKey, const char* zipPath )
{
	//MANA3DCurlRequest request;
	const char* appidptr = StringUtil::TCHAR_to_charPointer( appID );
	const char* secretptr = StringUtil::TCHAR_to_charPointer( secretKey );
	MANA3DCurlRequest::SubmitFile( appidptr, secretptr, zipPath );

	std::remove( zipPath );
}


void AMANA3DManager::SerializeAssetsData()
{
	if ( assetIDPairs == NULL ) 
	{
		GLog->Log( ELogVerbosity::Warning, "AssetIDPairs hasn't been initialized yet...." );
		return;
	}

	TSharedPtr<FJsonObject> mainJson = MakeShareable( new FJsonObject );

	// create Json values array
	TArray<TSharedPtr<FJsonValue>> meshArray;

	for ( MANA3DIDPair pair : assetIDPairs->meshIDPairs )
	{
		// create a Json object and add a string field
		TSharedPtr<FJsonObject> JsonObj = MakeShareable( new FJsonObject );

		JsonObj->SetStringField( pair.sourceID, FString::FromInt( pair.id ) );

		// create a Json value object to hold your Json object
		TSharedRef<FJsonValueObject> JsonValue = MakeShareable( new FJsonValueObject( JsonObj ) );

		// add the object to the array
		meshArray.Add( JsonValue );
	}
	
	mainJson->SetArrayField( "MeshIDPairs", meshArray );


	TArray<TSharedPtr<FJsonValue>> textureArray;

	for ( MANA3DIDPair pair : assetIDPairs->textureIDPairs )
	{
		// create a Json object and add a string field
		TSharedPtr<FJsonObject> JsonObj = MakeShareable( new FJsonObject );

		JsonObj->SetStringField( pair.sourceID, FString::FromInt( pair.id ) );

		// create a Json value object to hold your Json object
		TSharedRef<FJsonValueObject> JsonValue = MakeShareable( new FJsonValueObject( JsonObj ) );

		// add the object to the array
		textureArray.Add( JsonValue );
	}
	
	mainJson->SetArrayField( "TextureIDPairs", textureArray );


	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize( mainJson.ToSharedRef(), Writer );

	FFileHelper::SaveStringToFile( OutputString, *GetAssetIDPairsFilePath() );
}


int AMANA3DManager::AddMesh( UStaticMesh* mesh )
{
	FString sourceID = mesh->GetName();
	//mesh->GetMeshId( sourceID );

	int id = GetMeshID( sourceID );

	if ( assetIDPairs != NULL )
	{
		if ( id == -1 )
		{
			id = assetIDPairs->meshIDPairs.Num();
			if (id > 0)
				id = assetIDPairs->meshIDPairs[assetIDPairs->meshIDPairs.Num() - 1].id + 1;

			MANA3DIDPair meshIDPair( sourceID, id );
			assetIDPairs->meshIDPairs.Add( meshIDPair );
		}
		else
		{
			assetIDPairs->meshIDPairs[id].sourceID = sourceID;
		}
	}
	else
		GLog->Log( ELogVerbosity::Warning, "AssetIDPairs hasn't been initialized yet...." );

	return id;
}

int AMANA3DManager::AddTexture( UTexture2D* texture )
{
	FString sourceID = texture->GetName();//texture->Source.GetId().ToString();

	int id = GetTextureID( sourceID );

	if ( assetIDPairs != NULL )
	{
		if ( id == -1 )
		{
			id = assetIDPairs->textureIDPairs.Num();
			if (id > 0)
				id = assetIDPairs->textureIDPairs[assetIDPairs->textureIDPairs.Num() - 1].id + 1;

			MANA3DIDPair textureIDPair( sourceID, id );
			assetIDPairs->textureIDPairs.Add( textureIDPair );
		}
		else
		{
			assetIDPairs->textureIDPairs[id].sourceID = sourceID;
		}
	}
	else
		GLog->Log( ELogVerbosity::Warning, "AssetIDPairs hasn't been initialized yet...." );

	return id;
}

int AMANA3DManager::GetMeshID( FString sourceID )
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

int AMANA3DManager::GetTextureID( FString sourceID )
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
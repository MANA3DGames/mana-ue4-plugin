// Fill out your copyright notice in the Description page of Project Settings.

#include "UE4_MANA3D.h"
#include "MANA3DJob.h"
#include "SerializedData/MANA3DModel.h"
#include "MANA3DMeshIDPair.h"


MANA3DJob::MANA3DJob()
{
	data = NewObject<AMANA3DJobData>();
}

MANA3DJob::~MANA3DJob()
{
	if ( data != NULL )
		data->Destroy();
}


void MANA3DJob::AddModel( AActor* actor, AMANA3DAgent* agent )
{
	if ( actor == NULL )
		return;
	
	FTransform transform = actor->GetTransform();
	FMatrix matrix = transform.ToMatrixWithScale();

	// Get all UStaticMesh components associated with this actor.
	TArray<UStaticMeshComponent*> staticMeshComponents;
	actor->GetComponents<UStaticMeshComponent>( staticMeshComponents );
	for ( UStaticMeshComponent* smComponent : staticMeshComponents )
	{
		AMANA3DModel* model = NewObject<AMANA3DModel>();

		FString idstr = smComponent->StaticMesh->GetName();
		//smComponent->StaticMesh->GetMeshId( idstr );
		model->ObjID = FString::FromInt( data->models.Num() );
		model->meshID = FString::FromInt( agent->GetMeshID( idstr ) );
		model->materials = GetMaterials( smComponent->GetMaterials(), agent );

		for ( int i = 0; i < 4; i++ )
		{
			for ( int j = 0; j < 4; j++ )
				model->matrix[i][j] = matrix.M[i][j];
		}

		data->models.Add( model );
	}

	TArray<USkeletalMeshComponent*> skeletalMeshComponents;
	actor->GetComponents<USkeletalMeshComponent>( skeletalMeshComponents );
	for ( USkeletalMeshComponent* smComponent : skeletalMeshComponents )
	{
		AMANA3DModel* model = NewObject<AMANA3DModel>();
		
		model->meshID = "-1";
		
		int animatedModelID = data->additionalModels.Num();
		model->animatedModelID = animatedModelID;

		for ( int i = 0; i < 4; i++ )
		{
			for ( int j = 0; j < 4; j++ )
				model->matrix[i][j] = matrix.M[i][j];
		}

		model->materials = GetMaterials( smComponent->GetMaterials(), agent );
		model->ObjID = "a" + FString::FromInt( animatedModelID );

		TArray<uint32> indices;
		smComponent->GetSkeletalMeshResource()->LODModels[0].MultiSizeIndexContainer.GetIndexBuffer( indices );
		
		TArray<FVector> vertices;
		//smComponent->ComputeSkinnedPositions( vertices );		// Not all the position are correct!
		GetAnimatedVertexPositions( smComponent, vertices, true );

		FSkeletalMeshVertexBuffer& buffer = smComponent->GetSkeletalMeshResource()->LODModels[0].VertexBufferMANA3DUSkin;
		//FStaticLODModel& sModel = smComponent->SkeletalMesh->GetSourceModel();

		//int vCount = buffer.GetNumVertices();
		//for ( int i = 0; i < vCount; i++ )
		//{
		//	FVector vertex = buffer.GetVertexPositionSlow( i );
		//	//GLog->Log( FString::Printf( TEXT("%f %f %f			%f %f %f"), vertex.X, vertex.Y, vertex.Z, vertices[i].X, vertices[i].Y, vertices[i].Z ) );
		//	const FVector WorldSpaceVertexLocation = actor->GetActorLocation() + actor->GetTransform().TransformVector( vertex );
		//	//DrawDebuMANA3Doint( actor->GetWorld(), WorldSpaceVertexLocation, 10.0f, FColor::Red, false, 1000000 );
		//	vertices.Add( WorldSpaceVertexLocation );
		//}

		model->meshData = MeshToString( buffer, vertices, indices, 
										smComponent->SkeletalMesh->GetName(),
										false, true, model->animatedModelID );

		data->additionalModels.Add( model );
	}
}

FString MANA3DJob::GetMaterials( TArray<UMaterialInterface*> mats, AMANA3DAgent* MANA3DAgent )
{
	TMap<FString, FMANA3DMaterial> materialList;

	int index = 0;
	for ( UMaterialInterface* mat : mats )
	{
		FMANA3DMaterial objMaterial;

		objMaterial.name = "M" + FString::FromInt( index );

		FLinearColor lColor;
		if ( mat->GetVectorParameterValue( FName( "Color" ), lColor ) )
			objMaterial.color = lColor;
		else
			objMaterial.color = FLinearColor::White;

		TArray<UTexture*> textures;
		mat->GetUsedTextures( textures, EMaterialQualityLevel::High, true, ERHIFeatureLevel::Type::ES2, true );
		if ( textures.Num() > 0 )
		{
			for ( UTexture* tex : textures )
			{
				FString sID = tex->GetName(); //dynamic_cast<UTexture2D*>(tex)->Source.GetId().ToString();
				objMaterial.textureName = FString( "textures/" ).Append( FString::FromInt( MANA3DAgent->GetTextureID( sID ) ) ).Append( ".png" );
				objMaterial.textureID = MANA3DAgent->GetTextureID( sID );
			}
		}
		else
			objMaterial.textureName = NULL;

		materialList.Add( objMaterial.name, objMaterial );

		index++;
	}
	
	FString materials;

	TArray<FString, FDefaultAllocator> keys;
	materialList.GetKeys( keys );
	for (TKeyValuePair<FString, FDefaultAllocator> key : keys)
		materials.Append( GetMaterialString( key.Key, materialList[key.Key] ) );

	return materials;
}

FString MANA3DJob::GetMaterialString( FString name, FMANA3DMaterial material )
{
	FString str;

	str.Append( "newmtl " ).Append( name ).Append( "\n" );
	str.Append( FString::Printf( TEXT( "Ka  %f %f %f\n" ), material.color.R, material.color.G, material.color.B ) );
	str.Append( FString::Printf( TEXT( "Kd  %f %f %f\n" ), material.color.R, material.color.G, material.color.B ) );
	str.Append( "Ks  0 0 0\n" );
	str.Append( "d  1.0\n" );
	str.Append( "Ns  0\n" );
	str.Append( "Ni  10\n" );
	str.Append( "illum 0\n" );

	if ( !material.textureName.IsEmpty() )
		str.Append( FString::Printf( TEXT( "map_Kd %i.png" ), material.textureID ) );

	str.Append( "\n\n\n" );

	return str;
}

FString MANA3DJob::MeshToString( FSkeletalMeshVertexBuffer& mesh, TArray<FVector>& vertices, TArray<uint32>& indices,
							 FString name, bool invert, bool includeMaterials, int customMaterialID )
{
	FString sb;

    float factor = invert ? -1 : 1;

    if ( includeMaterials )
    {
        sb.Append( "\n" );
        sb.Append( "mtllib " );
        sb.Append("");
        if ( customMaterialID == -1 )
        {
            // MATERIAL_FILE to be replaced by the material used by the instance not the prefab.
            sb.Append( "MATERIAL_FILE.mtl" ).Append("\n" );
        }
        else
        {
            sb.Append( "a" + FString::FromInt( customMaterialID ) + ".obj.mtl" ).Append( "\n" );
        }
    }

	int verCount = vertices.Num();
    for ( int i = 0; i < verCount; i++ )
		sb.Append( FString::Printf( TEXT( "v %f %f %f\n" ), factor * vertices[i].X, vertices[i].Y, vertices[i].Z ) );
    sb.Append( "\n" );


    /*for ( FVector lv : mesh.normals )
    {
        Vector3 wv = lv;
        sb.Append( string.Format("vn {0} {1} {2}\n", factor * wv.x, wv.y, wv.z));
    }
    sb.Append("\n");*/
	

	verCount = mesh.GetNumVertices();
    for ( int i = 0; i < verCount; i++ )
    {
		FVector2D uv = mesh.GetVertexUV( i, 0 );
		sb.Append( FString::Printf( TEXT( "vt %f %f\n" ), uv.X, uv.Y ) );
    }

    sb.Append( "\n" );
    sb.Append( "g " ).Append( "model " );
	sb.Append( name );
	sb.Append( "\n" );
    sb.Append( "\n" );


	/*int sectionIndex = 0;
	int triCount = sModel.Sections[0].NumTriangles;
	sb.Append( "usemtl " ).Append( FString::Printf( TEXT( "M%i" ), sModel.Sections[sectionIndex].MaterialIndex ) ).Append( "\n" );*/

	for ( int i = 0; i < indices.Num(); i += 3 )
	{
		/*if ( triCount == 0 )
		{
			sectionIndex++;
			if ( sModel.Sections.Num() > sectionIndex )
			{
				sb.Append( "usemtl " ).Append( FString::Printf( TEXT( "M%i" ), sModel.Sections[sectionIndex].MaterialIndex ) ).Append( "\n" );
				triCount = sModel.Sections[sectionIndex].NumTriangles;
			}
		}
		triCount--;*/

		if ( invert )
		{
			sb.Append(FString::Printf(TEXT("f %i/%i/%i %i/%i/%i %i/%i/%i\n"), 
				indices[i + 1] + 1, indices[i + 1] + 1, indices[i + 1] + 1,
				indices[i] + 1,		indices[i] + 1,		indices[i] + 1,
				indices[i + 2] + 1, indices[i + 2] + 1, indices[i + 2] + 1));
		}
		else
		{
			sb.Append(FString::Printf(TEXT("f %i/%i/%i %i/%i/%i %i/%i/%i\n"), 
				indices[i + 2] + 1, indices[i + 2] + 1, indices[i + 2] + 1,
				indices[i] + 1,		indices[i] + 1,		indices[i] + 1,
				indices[i + 1] + 1, indices[i + 1] + 1, indices[i + 1] + 1));
		}
	}

	FString objPath = FString::Printf( TEXT( "%sSkelMesh.obj" ), *AMANA3DAgent::GetTempFilesPath() );
	FFileHelper::SaveStringToFile( sb, *objPath );

    return sb;
}


bool MANA3DJob::GetAnimatedVertexPositions( USkeletalMeshComponent* Mesh, 
										TArray<FVector>& vertices,
										bool PerformPawnVelocityCorrection )
{
	if( !Mesh || !Mesh->SkeletalMesh )  
		return false;
 
	vertices.Empty();
 
	Mesh->ComputeSkinnedPositions( vertices );
 
	FTransform ToWorld = Mesh->GetComponentTransform();
	FVector WorldLocation = ToWorld.GetLocation();
 
	//Pawn Velocity Correction
	UPawnMovementComponent* MovementComp = nullptr;
	if(PerformPawnVelocityCorrection)
	{
		APawn* Pawn = Cast<APawn>(Mesh->GetOwner());
		MovementComp = (Pawn) ? Pawn->GetMovementComponent() : NULL;
	}
	bool DoVelocityCorrection = PerformPawnVelocityCorrection && MovementComp;
	//Pawn Velocity Correction
 
	for ( FVector& EachVertex : vertices )
	{
		EachVertex = WorldLocation + ToWorld.TransformVector( EachVertex );
		if( DoVelocityCorrection )
		{
			EachVertex += MovementComp->Velocity * FApp::GetDeltaTime();
		} 
	} 
 
	return true;
}
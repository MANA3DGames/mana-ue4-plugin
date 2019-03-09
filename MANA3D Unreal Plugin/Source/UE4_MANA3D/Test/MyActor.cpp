// Fill out your copyright notice in the Description page of Project Settings.

#include "UE4_MANA3D.h"
#include "MyActor.h"


// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMyActor::Destroyed()
{
	if ( agent != NULL )
		agent->Destroy();
}

// Called every frame
void AMyActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}


void AMyActor::CreateAgent()
{
	if ( agent == NULL )
	{
		agent = GetWorld()->SpawnActor<AMANA3DAgent>( AMANA3DAgent::StaticClass() );
		agent->AppID = FText::FromString( "QK1bO2qNAkrZQD5CBtRj" );
		agent->SecretKey = FText::FromString( "IbMKbM6UgQZ6A1io4JcF" );
	}
	
	agent->Print( "test", "mahmoud@mxd3d.com" );
}

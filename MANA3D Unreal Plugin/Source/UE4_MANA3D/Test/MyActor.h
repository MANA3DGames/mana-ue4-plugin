// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MANA3DRuntime/MANA3DAgent.h"
#include "MyActor.generated.h"

UCLASS()
class UE4_MANA3D_API AMyActor : public AActor
{
	GENERATED_BODY()
	
private:
	AMANA3DAgent* agent;

public:	
	// Sets default values for this actor's properties
	AMyActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Destroyed() override;
	

	UFUNCTION(BlueprintCallable, Category = "MXD3D-MANA3D")
	void CreateAgent();
	
};

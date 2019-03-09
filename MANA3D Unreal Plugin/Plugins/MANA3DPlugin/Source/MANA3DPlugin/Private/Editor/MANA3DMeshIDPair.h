#pragma once

class MANA3DMeshIDPair
{
public:
	MANA3DMeshIDPair() {}
	~MANA3DMeshIDPair() {}

	FString meshName;
	FSkeletalMeshVertexBuffer meshBuffer;

	int id;

	bool requireUpdate = false;
};

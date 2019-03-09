#pragma once

class MANA3DIDPair
{
public:
	FString sourceID;
	int id;

	MANA3DIDPair( FString uSourceID, int uid )
	{
		sourceID = uSourceID;
		id = uid;
	}
};
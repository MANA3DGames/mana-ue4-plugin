#pragma once

#include "MANA3DManager.h"


class MANA3DWindow : public SCompoundWidget
{
private:
	// A pointer for MANA3DManager instance.
	AMANA3DManager* MANA3DManager;

	// Create a new MANA3DManager if there is no instance of MANA3DManager.
	void CreateMANA3DManagerInstance();

	// Triggered when user clicks on Bake button.
	FReply OnClickBakeAssets();

	// Just for testing.
	FReply OnClickViewActor();

public:
	// Default constructor.
	MANA3DWindow();

	// Default destructor.
	~MANA3DWindow();

	// Slate area.
	SLATE_BEGIN_ARGS( MANA3DWindow )
	{
	}
	SLATE_END_ARGS()

	// Override Construct method to create the plugin UI.
	void Construct( const FArguments& args );
};
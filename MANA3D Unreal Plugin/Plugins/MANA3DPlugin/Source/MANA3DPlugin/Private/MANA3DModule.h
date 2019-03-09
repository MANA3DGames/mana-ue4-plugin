#pragma once

DECLARE_LOG_CATEGORY_EXTERN(ModuleLog, Log, All)

class MANA3DModule : public IModuleInterface
{
private:
	// Create MANA3D widnow
	void CreateMANA3DWindowMenu( FMenuBuilder& menuBuilder );
	
	// Open MANA3D window.
	void OpenMANA3DWindow();

	// Create MANA3D tab.
	TSharedRef<SDockTab> CreateTab( const FSpawnTabArgs& args );

public:
	// Defualt constructor.
	MANA3DModule();

	// Will be triggered on module startup.
	virtual void StartupModule() override;

	// Will be triggered when module shutdown.
	virtual void ShutdownModule() override;


	// Returns singleton instance.
	static inline MANA3DModule& Get()
	{
		return FModuleManager::LoadModuleChecked<MANA3DModule>( "MANA3DModule" );
	}

	// Checks to see if this module is loaded and ready.
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "MANA3DModule" );
	}
};
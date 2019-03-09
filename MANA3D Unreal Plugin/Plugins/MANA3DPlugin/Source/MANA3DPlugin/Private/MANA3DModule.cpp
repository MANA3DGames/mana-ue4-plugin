#include "MANA3DPluginPch.h"
#include "MANA3DModule.h"
#include "Editor/MANA3DWindow.h"

IMPLEMENT_MODULE(MANA3DModule, MANA3DPlugin)
DEFINE_LOG_CATEGORY(ModuleLog)

#define LOCTEXT_NAMESPACE "MANA3DPlugin"

// Create a constant name.
const FName MANA3DWindowTabName( TEXT( "MANA3DWindowTab" ) );


MANA3DModule::MANA3DModule()
{
}


//Edit-> Editor Preferences -> Miscellaneous -> Display UIExtension Points. 
void MANA3DModule::StartupModule()
{
	// Registering the window button.
	TSharedPtr<FExtender> extender = MakeShareable( new FExtender );
	extender->AddMenuExtension(
		"General",
		EExtensionHook::After,
		NULL,
		FMenuExtensionDelegate::CreateRaw( this, &MANA3DModule::CreateMANA3DWindowMenu )
	);

	FLevelEditorModule& levelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
	levelEditor.GetMenuExtensibilityManager()->AddExtender( extender );

	// Registering the tab.
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		MANA3DWindowTabName,
		FOnSpawnTab::CreateRaw(this, &MANA3DModule::CreateTab)
	)
	.SetDisplayName( FText::FromString( TEXT("MANA3D Window") ) );
}

void MANA3DModule::ShutdownModule()
{
	//FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MANA3DWindowTabName);
}


void MANA3DModule::CreateMANA3DWindowMenu( FMenuBuilder& menuBuilder )
{
	menuBuilder.AddMenuEntry(
		LOCTEXT( "OpenMANA3DWindow", "MANA3D" ),
		LOCTEXT( "OpenMANA3DWindowTooltip", "Opens MANA3D plugin window" ),
		FSlateIcon(),
		FUIAction( FExecuteAction::CreateRaw( this, &MANA3DModule::OpenMANA3DWindow ) )
	);
}

void MANA3DModule::OpenMANA3DWindow()
{
	FGlobalTabmanager::Get()->InvokeTab( MANA3DWindowTabName );
}


TSharedRef<SDockTab> MANA3DModule::CreateTab( const FSpawnTabArgs& args )
{
	return 
		SNew( SDockTab )
		.Label( LOCTEXT( "TabTitle", "MANA3D" ) )
		.TabRole( ETabRole::MajorTab )
		.ContentPadding( 5 )
		[
			SNew( SBorder )
			.Padding(4)
			.BorderImage( FEditorStyle::GetBrush( "ToolPanel.GroupBorder" ) )
			[
				SNew( MANA3DWindow )
			]
		];
}
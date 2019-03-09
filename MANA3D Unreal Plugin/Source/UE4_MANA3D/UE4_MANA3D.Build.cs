// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class UE4_MANA3D : ModuleRules
{
	public UE4_MANA3D(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "InputCore" } );

		PrivateDependencyModuleNames.AddRange( new string[] {  } );

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");
        // if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
        // {
        //		if (UEBuildConfiguration.bCompileSteamOSS == true)
        //		{
        //			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        //		}
        // }


        #region [Include MANA3DModule]
        var projectPath = System.IO.Path.GetFullPath( ModuleDirectory );
        var projectName = string.Empty;
        for ( int i = 0; i < 2; i++ )
        {
            projectPath = System.IO.Directory.GetParent( projectPath ).FullName;
            if ( i < 3 )
                projectName = System.IO.Directory.GetParent( projectPath ).Name;
        }

        string privateSource = System.IO.Path.Combine( projectPath, @"Plugins\MANA3DPlugin\Source\MANA3DPlugin\Private" );
        string publicSource = System.IO.Path.Combine( projectPath, @"Plugins\MANA3DPlugin\Source\MANA3DPlugin\Public" );

        PublicIncludePaths.Add( privateSource );
        PublicIncludePaths.Add( System.IO.Path.Combine( privateSource, "Editor" ) );
        PublicIncludePaths.Add( System.IO.Path.Combine( privateSource, "Utilities" ) );
        PublicIncludePaths.Add( System.IO.Path.Combine( publicSource, "includes" ) );
        PublicIncludePaths.Add( System.IO.Path.Combine( publicSource, @"includes\zlib" ) );
  
		PublicAdditionalLibraries.Add( System.IO.Path.Combine( publicSource, @"lib\libcurl.Lib" ) );
        PublicDependencyModuleNames.AddRange( new string[] { "Json", "JsonUtilities", "zlib", "libcurl" } );
        #endregion
    }
}

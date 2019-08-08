// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BigNanto : ModuleRules
{
	public BigNanto(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Networking", "Sockets", "Paper2D" }); // UMG, Networking, Socket �߰�


        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "Paper2D" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

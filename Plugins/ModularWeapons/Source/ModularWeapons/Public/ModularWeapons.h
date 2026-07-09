// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** Log category for the entire Modular Weapons system. */
MODULARWEAPONS_API DECLARE_LOG_CATEGORY_EXTERN(LogModularWeapons, Log, All);

/**
 * Runtime module entry point for the Modular Weapons plugin.
 *
 * The plugin has no global state; the module implementation exists only to
 * register the log category and satisfy the engine's module contract.
 */
class FModularWeaponsModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface
};

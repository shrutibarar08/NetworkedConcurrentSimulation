#pragma once

#include "FileManager/FileLoader/SweetLoader.h"

class ISystem
{
public:
    ISystem() = default;
    virtual ~ISystem() = default;

    ISystem(const ISystem&) = delete;
    ISystem(ISystem&&) = delete;
    ISystem& operator=(const ISystem&) = delete;
    ISystem& operator=(ISystem&&) = delete;

    // Load initial state/config from some ConfigManager
    virtual bool BuildFromConfig(const SweetLoader* sweetLoader) = 0;

	virtual bool Init() = 0;
    virtual bool Run() = 0;
    virtual bool Shutdown() = 0;
};

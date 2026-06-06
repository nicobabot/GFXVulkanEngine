#pragma once
#include "Dvar.h"
class GfxOptions
{
public:
    static GfxOptions& get()
    {
        static GfxOptions instance;
        return instance;
    }

    GfxOptions(const GfxOptions&) = delete;
    GfxOptions& operator=(const GfxOptions&) = delete;

private:
    GfxOptions() = default;
public:
	bool isOpen = true;
	Dvar<bool> blurEnabled;
};
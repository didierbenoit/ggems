#pragma once
// ************************************************************************
// ************************************************************************


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <io.h>
#include <windows.h>
#include <winternl.h>
#include <Psapi.h>
#include <conio.h>

#define isatty _isatty

#define fileno _fileno
#endif

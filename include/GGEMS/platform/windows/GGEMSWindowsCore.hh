#pragma once

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

/// \cond
#include <io.h>
#include <windows.h>
#include <winternl.h>
/// \endcond

#define isatty _isatty
#define fileno _fileno

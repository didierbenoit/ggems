#pragma once

#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"

// On masque les warnings externes
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

/// \cond
#include <d3dkmthk.h> // Appelle D3DKMTQueryStatistics, QueryNode/Node2
/// \endcond

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

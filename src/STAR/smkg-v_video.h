// SONIC ROBO BLAST 2; TSOURDT3RD
//-----------------------------------------------------------------------------
// Copyright (C) 2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  smkg-v_video.h
/// \brief Globalizes TSoURDt3rd's exclusive video flags and routines

#ifndef __SMKG_V_VIDEO__
#define __SMKG_V_VIDEO__

#include "smkg-cvars.h"

// ------------------------ //
//        Variables
// ------------------------ //

// ===================================================================
// DYNAMIC VIDEO COLORS
//
// 	Remember this from some previous TSoURDt3rd commits of st_stuff.c?
// 		Yeah, I reworked it! Just like I said I would!
//		But this time, I've reworked it even more!
// ===================================================================

#define V_MENUCOLORMAP (cv_tsourdt3rd_video_coloring_menus.value)
#define V_FPSCOLORMAP  (cv_tsourdt3rd_video_coloring_fpsrate.value)
#define V_TPSCOLORMAP  (cv_tsourdt3rd_video_coloring_tpsrate.value)

#endif // __SMKG_V_VIDEO__

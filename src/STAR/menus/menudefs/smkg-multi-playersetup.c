// SONIC ROBO BLAST 2; TSOURDT3RD
//-----------------------------------------------------------------------------
// Original Copyright (C) 2018-2020 by Sally "TehRealSalt" Cochenour.
// Original Copyright (C) 2018-2024 by Kart Krew.
// Copyright (C) 2020-2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  menus/menudefs/smkg-multi-playersetup.c
/// \brief Modifying data for the player setup menu.

#include "../../m_menu.h"
#include "../../p_user.h"

#include "../../../g_game.h"
#include "../../../v_video.h"

// ------------------------ //
//        Functions
// ------------------------ //

//
// void TSoURDt3rd_M_DrawColorResetOption
// (
//	INT32 x, INT32 *y, *INT32 cursory,
//	player_t *setupm_player,
//	consvar_t *setupm_cvdefaultskin, consvar_t *setupm_cvdefaultcolor, menucolor_t *setupm_fakecolor
// )
//
// Draws the 'Reset to Defaults' option in the Player Setup menu.
//
void TSoURDt3rd_M_DrawColorResetOption
(
	INT32 x, INT32 *y, INT32 *cursory,
	player_t *setupm_player,
	consvar_t *setupm_cvdefaultskin, consvar_t *setupm_cvdefaultcolor, menucolor_t *setupm_fakecolor
)
{
	*y += 10;
	if (tsourdt3rd_itemOn == 4)
		*cursory = *y;

	V_DrawString(x, *y,
		((R_SkinAvailable(skins[setupm_player->skin].name) != setupm_cvdefaultskin->value
			|| setupm_cvdefaultcolor->value != setupm_fakecolor->color)
		? 0
		: V_TRANSLUCENT)| ((tsourdt3rd_itemOn == 4) ? V_MENUCOLORMAP : 0),
		"Reset to defaults");
}

//
// void TSoURDt3rd_M_HandleColorResetOption
// (
//	player_t *setupm_player,
//	INT32 *setupm_fakeskin, consvar_t *setupm_cvdefaultskin,
//	consvar_t *setupm_cvdefaultcolor, menucolor_t *setupm_fakecolor
// )
//
// Handles choices for the 'Reset to Defaults' option in the Player Setup menu.
//
void TSoURDt3rd_M_HandleColorResetOption
(
	player_t *setupm_player,
	INT32 *setupm_fakeskin, consvar_t *setupm_cvdefaultskin,
	consvar_t *setupm_cvdefaultcolor, menucolor_t *setupm_fakecolor
)
{
	INT32 play_sound = sfx_strpst, playernum = (setupm_player == &players[consoleplayer] ? consoleplayer : secondarydisplayplayer);
	boolean reset_skin = true, reset_color = true;

	if (tsourdt3rd_itemOn != 4 || !setupm_player)
		return;

	if ((R_SkinAvailable(skins[setupm_player->skin].name) == setupm_cvdefaultskin->value || *setupm_fakeskin == setupm_cvdefaultskin->value)
	&& setupm_cvdefaultcolor->value == setupm_fakecolor->color)
	{
		// Everything's default here, let's just leave...
		STAR_CONS_Printf(STAR_CONS_TSOURDT3RD_ALERT, "Your skin and color are already set to the defaults!\n");
		return;
	}

	if (gametyperules & GTR_TEAMS && !setupm_player->spectator)
	{
		reset_color = false;
		reset_skin = TSoURDt3rd_P_MovingPlayerSetup(playernum);
		setupm_fakecolor->color = (setupm_player->ctfteam == 1 ? skincolor_redteam : skincolor_blueteam);
	}
	else if (!TSoURDt3rd_P_MovingPlayerSetup(playernum))
		reset_color = reset_skin = false;

	if (reset_skin)
	{
		*setupm_fakeskin = R_SkinAvailable(setupm_cvdefaultskin->string);
		if (setupm_player->mo && R_SkinAvailable(skins[setupm_player->skin].name) != setupm_cvdefaultskin->value)
			setupm_player->mo->skin = (skin_t *)&skins[*setupm_fakeskin];
		STAR_CONS_Printf(STAR_CONS_TSOURDT3RD_NOTICE, "Your skin has been reset to the default!\n");
	}
	else
	{
		play_sound = sfx_skid;
		STAR_CONS_Printf(STAR_CONS_TSOURDT3RD_NOTICE, "Your skin can't be reset right now!\n");
	}

	if (reset_color)
	{
		setupm_fakecolor->color = setupm_cvdefaultcolor->value;
		STAR_CONS_Printf(STAR_CONS_TSOURDT3RD_NOTICE, "Your color has been reset to the default!\n");
	}
	else
	{
		play_sound = sfx_skid;
		STAR_CONS_Printf(STAR_CONS_TSOURDT3RD_NOTICE, "Your color can't be reset right now!\n");
	}

	S_StartSound(NULL, play_sound);
}
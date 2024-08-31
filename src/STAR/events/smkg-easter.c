// SONIC ROBO BLAST 2; TSOURDT3RD
//-----------------------------------------------------------------------------
// Copyright (C) 2024 by Star "Guy Who Names Scripts After Him" ManiaKG.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  smkg-easter.c
/// \brief Easter event related functions and data

#include "../ss_main.h"
#include "../star_vars.h"
#include "../smkg-st_hud.h"
#include "../m_menu.h"

#include "../../d_main.h" // autoloaded //
#include "../../g_game.h" // playeringame[] //
#include "../../lua_hud.h"
#include "../../st_stuff.h"
#include "../../v_video.h"

// ------------------------ //
//        Variables
// ------------------------ //

INT32 currenteggs;
INT32 collectedmapeggs;
INT32 numMapEggs;

static void STAR_Easter_EggHunt_OnChange(void);
static void STAR_Easter_EnableEggHuntBonuses_OnChange(void);

// ------------------------ //
//        Commands
// ------------------------ //

consvar_t cv_easter_allowegghunt = CVAR_INIT ("easter_allowegghunt", "No", CV_SAVE|CV_CALL|CV_NOINIT, CV_YesNo, STAR_Easter_EggHunt_OnChange);
consvar_t cv_easter_egghuntbonuses = CVAR_INIT ("easter_egghuntbonuses", "Off", CV_SAVE|CV_CALL|CV_NOINIT, CV_OnOff, STAR_Easter_EnableEggHuntBonuses_OnChange);

// ------------------------ //
//        Functions
// ------------------------ //

boolean TSoURDt3rd_Easter_AllEggsCollected(void)
{
	return (EnableEasterEggHuntBonuses && currenteggs == TOTALEGGS);
}

static void STAR_Easter_EggHunt_OnChange(void)
{
	if (!eastermode)
		return;

	if ((Playing() && playeringame[consoleplayer]) || netgame || (TSoURDt3rd_NoMoreExtras || autoloaded) || currenteggs == TOTALEGGS)
	{
		if (TSoURDt3rd_NoMoreExtras || autoloaded)
			CONS_Printf("Sorry, you'll need to restart your game in order to set this again.\n");
		else if ((Playing() || gamestate == GS_TITLESCREEN || menuactive) && currenteggs == TOTALEGGS)
			CONS_Printf("You already have all of the eggs!\n");
		else if (Playing() && netgame)
			CONS_Printf("Sorry, you can't change this while in a game or netgame.\n");

		CV_StealthSetValue(&cv_easter_allowegghunt, !cv_easter_allowegghunt.value);
		return;
	}
}

static void STAR_Easter_EnableEggHuntBonuses_OnChange(void)
{
	if (eastermode)
		return;

	if ((netgame || currenteggs != TOTALEGGS) || (TSoURDt3rd_NoMoreExtras || autoloaded))
	{
		((TSoURDt3rd_NoMoreExtras || autoloaded) ?
			(CONS_Printf("Sorry, you'll need to restart your game in order to set this again.\n")) :

			(((Playing() || gamestate == GS_TITLESCREEN || menuactive) && currenteggs != TOTALEGGS) ?
				(CONS_Printf("You can't set this yet!\nYou didn't get all those easter eggs!\n")) :

			((Playing() && netgame) ?
				(CONS_Printf("Sorry, you can't change this while in a netgame.\n")) : 0)));

		CV_StealthSetValue(&cv_easter_egghuntbonuses, !cv_easter_egghuntbonuses.value);
	}
	else
		EnableEasterEggHuntBonuses = cv_easter_egghuntbonuses.value;
}

//
// void TSoURDt3rd_Easter_ST_drawEggs(void);
// Draws TSoURDt3rd's cool Easter HUD.
//
// STAR NOTE FOR SNOOPERS: THIS CONTAINS PIECES OF CODE THAT tsourdt3rd.pk3 INITIALIZES, SETS, AND RUNS.
//							IF YOU WANT TO SEE THE REST, JUST TAKE A LOOK AT THAT PK3.
//
void TSoURDt3rd_Easter_ST_drawEggs(void)
{
	// Run Some Checks
	if (!Playing() 										// We Need to Play, Jesse
		|| (netgame || multiplayer)						// You Can't Manipulate Your Friends for This Egg Hunt
		|| (!eastermode)								// We Shouldn't Even Show This If It's Not Easter
		|| (TSoURDt3rd_NoMoreExtras)					// No Cheating
		|| (autoloaded)									// No Cheating: Electric Boogalo
		|| (!AllowEasterEggHunt)						// Hooray for Consent
		
		|| (F_GetPromptHideHud(hudinfo[HUD_RINGS].y))	// If Rings are Hidden, So Are the Eggs
		|| (!LUA_HudEnabled(hud_rings)))					// Same as above

		return;

	//// NOW WE RENDER, AND WE'RE DONE! :) ////
	// Draw the Patches and Strings //
	if (numMapEggs && (collectedmapeggs != numMapEggs))
	{	
		// Map Eggs
		V_DrawScaledPatch(16, 64, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), tsourdt3rd_easter_leveleggs);

		V_DrawTallNum(115, 64, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), collectedmapeggs);
		V_DrawString(115, 64, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), "/");
		V_DrawTallNum(140, 64, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), numMapEggs);
		
		// Total Eggs
		V_DrawScaledPatch(16, 80, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), tsourdt3rd_easter_totaleggs);

		V_DrawTallNum(115, 80, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), currenteggs);
		V_DrawString(115, 80, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), "/");
		V_DrawTallNum(140, 80, V_SNAPTOLEFT|V_SNAPTOTOP|V_PERPLAYER|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS), TOTALEGGS);
	}

	// Draw the Egg Notifier //
	else
		V_DrawCenteredThinString(16, 64,
			((((currenteggs == TOTALEGGS) || (numMapEggs && (collectedmapeggs == numMapEggs))) ? (V_GREENMAP) : (V_REDMAP))|((stplyr->spectator) ? V_HUDTRANSHALF : V_HUDTRANS)),
			((currenteggs == TOTALEGGS) ? ("All Eggs Have Been Found!") : ((numMapEggs && (collectedmapeggs == numMapEggs)) ? ("All Eggs in this Map Have Been Found!") : ("There Are No Eggs in This Map!"))));
}
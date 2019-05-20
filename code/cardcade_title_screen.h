/*@H
* File: cardcade_title_screen.h
* Author: Jesse Calvert
* Created: May 21, 2018, 15:56
* Last modified: November 5, 2018, 13:42
*/

#pragma once

struct game_mode_title_screen
{
	v4 BackgroundColor;
	v4 TextColor;
};

internal void PlayTitleScreen(game_state *GameState);

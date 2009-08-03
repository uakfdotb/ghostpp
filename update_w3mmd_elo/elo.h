/* 
 * File: elo.h
 * Author: GGZ Dev Team
 * Project: GGZ Server (moved from ggzdmod/ggzstats)
 * Date: 5/07/2002 (moved from ggz_stats.c)
 * Desc: GGZ game module stat functions - ELO ratings
 * $Id: elo.h,v 1.1 2002/10/28 04:56:55 jdorje Exp $
 *
 * Copyright (C) 2001-2002 GGZ Dev Team.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/** Recalculate player rankings based on an ELO system, given a set of teams
 *  with pre-averaged rankings.
 *
 *  @param num_players The number of players involved.
 *  @param player_ratings A rating for each player.
 *  @param player_teams A list of which team each player is on.
 *  @param num_teams The number of teams.
 *  @param team_ratings A rating for each team.
 *  @param team_winners What portion of winning [0, 1] each team has done.
 *  @return Nothing; but the player_ratings array will be modified.
 *
 *  @note The teams must be specifically enumerated.  If there are no "teams",
 *  just have the array be identical to the player array.
 */
void elo_recalculate_ratings(int num_players, float *player_ratings,
			     int *player_teams, int num_teams,
			     float *team_ratings, float *team_winners);

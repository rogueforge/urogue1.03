/*
 * Last Modified: 05/25/86
 */
/*
 * This file contains functions for dealing with special player abilities
 * @(#)player.c	7.0 (Indian Hill) 10/5/82
 */

#include "curses.h"
#include "rogue.h"

/* Constitution bonus */

const_bonus()	/* Hit point adjustment for changing levels */
{
    if (pstats.s_const > 6 && pstats.s_const <= 12) 
	return(0);
    if (pstats.s_const > 12) 
	return(pstats.s_const-12);
    if (pstats.s_const > 3) 
	return(-1);
    return(-2);
}

/* Routines for everyone */

prayer() 	/* Pray to a deity */
{
register int
    chance, i, times;
char
    ch, num_str[20];
struct linked_list 
    *item;
struct thing 
    *tp;
bool
    got_one = FALSE;

    ++times_prayed;

    if (wizard) {
	msg("What roll?[0..%d] ",99);
	ch = get_str(num_str, cw);
	if (ch == ESCAPE) {
	    msg("");
	    return;
	}
	chance = atoi(num_str);
    } else {
        if (times_prayed > pstats.s_lvl && (!(player.t_ctype == C_DRUID ||
                                              player.t_ctype == C_CLERIC))) {
	    chance = 100;
        } else 
            chance = rnd(100);
    }

    switch (chance) {

    case 0:
        msg("Your prayers are answered.");	
	if (PRAYER_ABILITY) 
	    times = 8;
	else 
	    times = 1;
	/*
	 * kill all monsters surrounding the hero except unique ones 
	 * This will change when I implement the deity option.  If
	 * The deity is "stronger" than the unique monster, then
	 * the monster will be killed.
	 */
	 for (i = 0; i < times ; i++) { 
	    item = find_mons_around(player.t_pos.y, player.t_pos.x, TRUE);
	    if (item) {
	        tp = THINGPTR(item);
	        msg("A bolt of eldritch energy strikes down the %s!!!",
	        monsters[tp->t_index].m_name);
	        killed(NULL, item, FALSE, FALSE);
		got_one = TRUE;
	    }
	} 
	if (!got_one)
	   msg("Unfortunately, you can't determine just what was done.");

    when 10:
	msg("Your prayers are answered");
	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE))
            read_scroll(S_SUMMON, PRAYER_ABILITY);
	else
	    msg("Unfortunately, you can't determine just what was done.");

    when 20:
	msg("Your prayers are answered");
	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE))
	    read_scroll(S_TELEP, PRAYER_ABILITY);
	else
	    msg("Unfortunately, you can't determine just what was done.");

    when 30:
	msg("Your prayers are answered");
	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE))
            read_scroll(S_SLEEP, PRAYER_ABILITY);
	else
	    msg("Unfortunately, you can't determine just what was done.");

    when 40:
	msg("Your prayers are answered");
     	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE)) {
	    if (pstats.s_hpt <= max_stats.s_hpt) {
	        msg("You feel someone opening your mouth.");
	        quaff(P_RESTORE, PRAYER_ABILITY);
	        quaff(P_HEALING, PRAYER_ABILITY);
	    }
	} else
	    msg("Unfortunately, you can't determine just what was done.");

    when 50:
	msg("Your prayers are answered");
     	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE)) {
	    msg("A bolt of lighting strikes you.");
            msg("But for some reason, you aren't harmed");
	    read_scroll(S_ELECTRIFY, PRAYER_ABILITY);
	} else
	    msg("Unfortunately, you can't determine just what was done.");

    when 60:
	msg("Your prayers are answered");
     	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE)) 
	    read_scroll(S_HOLD, PRAYER_ABILITY);
	else
	    msg("Unfortunately, you can't determine just what was done.");
	
    when 70:
	msg("Your prayers are answered");
	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE))
	    read_scroll(S_MAKEIT, PRAYER_ABILITY);
	else
	    msg("Unfortunately, you can't determine just what was done.");

    when 80:
        msg("Your prayers are answered");
     	if (find_mons_around(player.t_pos.y, player.t_pos.x, TRUE)) 
	    quaff(P_DISGUISE, PRAYER_ABILITY);
	else
	    msg("Unfortunately, you can't determine just what was done.");
	    
    when 90:
	if (pstats.s_hpt<5) {
	    msg("I have been sent to protect you on this level.");
	    summon_monster(NUMMONST+2, FALSE, FALSE);
	}
	else
	    msg("Nothing happens");

    when 100:
	msg("A bolt of lighting zaps you");
	if (pstats.s_hpt > 1)
	    pstats.s_hpt >>= 1;
    break;

    default:
	msg("Nothing happens");


    }
}

/* Routines for thieves */

/*
 * gsense:
 *	Sense gold
 */

gsense()
{
    if (lvl_obj != NULL) {
	struct linked_list *gitem;
	struct object *cur;
	int gtotal = 0;

	wclear(hw);
	for (gitem = lvl_obj; gitem != NULL; gitem = next(gitem)) {
	    cur = (struct object *) ldata(gitem);
	    if (cur->o_type == GOLD) {
		gtotal += cur->o_count;
		mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, GOLD);
	    }
	}
	if (gtotal) {
	    s_know[S_GFIND] = TRUE;
	    msg("You sense gold!");
	    overlay(hw,cw);
	    return;
	}
    }
    msg("You can sense no gold on this level.");
}


/*
 * steal:
 *	Steal in direction given in delta
 */

steal()
{
    register struct linked_list *item;
    register struct thing *tp;
    register char *mname;
    coord new_pos;
    short thief_bonus = -50;

    new_pos.y = hero.y + delta.y;
    new_pos.x = hero.x + delta.x;

    /* Anything there? */
    if (new_pos.y < 0 || new_pos.y > LINES-3 ||
	new_pos.x < 0 || new_pos.x > COLS-1 ||
	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
	msg("Nothing to steal from.");
	return;
    }

    if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
	debug("Steal from what @ %d,%d?", new_pos.y, new_pos.x);
	return;
    }
    tp = THINGPTR(item);
    mname = monsters[tp->t_index].m_name;

    /* Can player steal something unnoticed? */
    if (player.t_ctype == C_THIEF) thief_bonus = 10;

    if (rnd(100) <
	(thief_bonus + 2*pstats.s_dext + 5*pstats.s_lvl -
	 5*(tp->t_stats.s_lvl - 3))) {
	register struct linked_list *s_item, *pack_ptr;
	short count = 0;

	s_item = NULL;	/* Start stolen goods out as nothing */

	/* Find a good item to take */
	for (pack_ptr=tp->t_pack; pack_ptr != NULL; pack_ptr=next(pack_ptr))
	    if (rnd(++count) == 0) s_item = pack_ptr;

	/* if we have a merchant, get an item */
	if (s_item == NULL && tp->t_index == NUMMONST)
	    s_item = new_thing();


	/* Find anything? */
	if (s_item == NULL) {
	    msg("The %s has nothing to steal.", mname);
	    return;
	}

	/* Take it from monster */
	if (tp->t_pack) detach(tp->t_pack, s_item);

	/* Give it to player */
	if (add_pack(s_item, FALSE) == FALSE) {
	   (OBJPTR(s_item))->o_pos = hero;
	   fall(&player, s_item, TRUE);
	}

	/* Get points for stealing */
	pstats.s_exp += tp->t_stats.s_exp/2;

	/*
	 * Do adjustments if player went up a level
	 */
	check_level();
    }

    else {
	msg("Your attempt fails.");

	/* Annoy monster (maybe) */
	if (rnd(35) >= pstats.s_dext + thief_bonus) runto(&new_pos, &hero);
    }
}


/* Routines for clerics */


cleric_pray()
{
    register short i, num_prayers, which_prayer, avail_points;
    register bool nohw = FALSE;

    /* Get the number of avilable prayers */
    if (pstats.s_wisdom > 16) 
	num_prayers = (pstats.s_wisdom - 15) / 2;
    else 
	num_prayers = 0;

    if (player.t_ctype == C_CLERIC || player.t_ctype == C_DRUID) 
	num_prayers += pstats.s_lvl;
    else if (player.t_ctype == C_PALADIN && pstats.s_lvl > 8)
	num_prayers += pstats.s_lvl - 5;

    if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC &&
	player.t_ctype != C_DRUID && ISWEARING(R_WIZARD))
		num_prayers *= 2;

    if (num_prayers > MAXPRAYERS_C) 
	num_prayers = MAXPRAYERS_C;

    avail_points = pstats.s_wisdom * pstats.s_lvl;
    if(ISWEARING(R_WIZARD))
	avail_points *= 2;

    /* Prompt for prayer */
    msg(""); /* Kludge to rid us of irritating -- More -- */
    msg("Which prayer are you offering [%d points left] (* for list)? ",
			avail_points - spell_power);
    mpos=0;

    which_prayer = (short) ((getchar() & 0177) - 'a');
    if (which_prayer == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which_prayer >= 0 && which_prayer < num_prayers) 
	nohw = TRUE;
    else {
	register char c;
	char buf[LINELEN];
	char *buf2;

	for (i=0; i<num_prayers; i++) {
	    sprintf(buf, "[%c, %d/%d] %s", 
		        (char) ((short) 'a' + i),
		        cleric_spells[i].s_cost,
			avail_points - spell_power,
			(terse) ? "" : "A prayer for ");
	    if (cleric_spells[i].s_type == TYP_POTION)
		buf2 = p_magic[cleric_spells[i].s_which].mi_name;
	    else if (cleric_spells[i].s_type == TYP_SCROLL)
		buf2 = s_magic[cleric_spells[i].s_which].mi_name;
	    else if (cleric_spells[i].s_type == TYP_STICK)
		buf2 = ws_magic[cleric_spells[i].s_which].mi_name;
	    strcat(buf, buf2);
	    if(num_prayers == 1)
		msg(buf);
	    else
		add_line(buf);
	}
	if(num_prayers != 1)
		end_line();
        msg(""); /* Kludge to rid us of irritating -- More -- */
	msg("Which prayer are you offering? ");
    }

    if (!nohw) {
	which_prayer = (short) ((getchar() & 0177) - 'a');
	while (which_prayer < 0 || which_prayer >= num_prayers) {
	    if (which_prayer == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("Please enter one of the listed prayers: ");
	    which_prayer = (short) ((getchar() & 0177) - 'a');
	}
    }


    if ((cleric_spells[which_prayer].s_cost + spell_power) > avail_points) {
	msg("Your prayer fails.");
	return;
    }

    msg("Your prayer is successful.");
    if (cleric_spells[which_prayer].s_type == TYP_POTION)
	quaff(	cleric_spells[which_prayer].s_which,
		cleric_spells[which_prayer].s_blessed);
    else
	read_scroll(	cleric_spells[which_prayer].s_which,
			cleric_spells[which_prayer].s_blessed);

    spell_power += cleric_spells[which_prayer].s_cost;
}



/*
 * Routines for Druids
 */
druid_pray()
{
    register short i, num_prayers, which_prayer, avail_points;
    register bool nohw = FALSE;

    /* Get the number of avilable prayers */
    if (pstats.s_wisdom > 16) 
	num_prayers = (pstats.s_wisdom - 15) / 2;
    else 
	num_prayers = 0;

    if (player.t_ctype == C_DRUID) 
	num_prayers += pstats.s_lvl;
    else if (player.t_ctype == C_PALADIN && pstats.s_lvl > 8)
	num_prayers += pstats.s_lvl - 5;

    if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC &&
	player.t_ctype != C_DRUID && ISWEARING(R_WIZARD))
		num_prayers *= 2;

    if (num_prayers > MAXPRAYERS_D) 
	num_prayers = MAXPRAYERS_D;

    avail_points = pstats.s_wisdom * pstats.s_lvl;
    if(ISWEARING(R_WIZARD))
	avail_points *= 2;

    /* Prompt for prayer */
    msg(""); /* Kludge to rid us of irritating -- More -- */
    msg("Which prayer are you offering [%d points left] (* for list)? ",
			avail_points - spell_power);
    mpos=0;

    which_prayer = (short) ((getchar() & 0177) - 'a');
    if (which_prayer == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which_prayer >= 0 && which_prayer < num_prayers) 
	nohw = TRUE;
    else {
	register char c;
	char buf[LINELEN];
	char *buf2;

	for (i=0; i<num_prayers; i++) {
	    sprintf(buf, "[%c, %d/%d] %s", 
		        (char) ((short) 'a' + i),
		        druid_spells[i].s_cost,
			avail_points - spell_power,
			(terse) ? "" : "A prayer for ");
	    if (druid_spells[i].s_type == TYP_POTION)
		buf2 = p_magic[druid_spells[i].s_which].mi_name;
	    else if (druid_spells[i].s_type == TYP_SCROLL)
		buf2 = s_magic[druid_spells[i].s_which].mi_name;
	    else if (druid_spells[i].s_type == TYP_STICK)
		buf2 = ws_magic[druid_spells[i].s_which].mi_name;
	    strcat(buf, buf2);
	    if(num_prayers == 1)
		msg(buf);
	    else
		add_line(buf);
	}
	if(num_prayers != 1)
		end_line();
        msg(""); /* Kludge to rid us of irritating -- More -- */
	msg("Which prayer are you offering? ");
    }

    if (!nohw) {
	which_prayer = (short) ((getchar() & 0177) - 'a');
	while (which_prayer < 0 || which_prayer >= num_prayers) {
	    if (which_prayer == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("Please enter one of the listed prayers: ");
	    which_prayer = (short) ((getchar() & 0177) - 'a');
	}
    }


    if ((druid_spells[which_prayer].s_cost + spell_power) > avail_points) {
	msg("Your prayer fails.");
	return;
    }

    msg("Your prayer is successful.");
    if (druid_spells[which_prayer].s_type == TYP_POTION)
	quaff(	druid_spells[which_prayer].s_which,
		druid_spells[which_prayer].s_blessed);
    else
	read_scroll(	druid_spells[which_prayer].s_which,
			druid_spells[which_prayer].s_blessed);

    spell_power += druid_spells[which_prayer].s_cost;
}


/*
 * affect:
 *	cleric affecting undead
 */

affect()
{
    register struct linked_list *item;
    register struct thing *tp;
    register char *mname;
    coord new_pos;

    if (player.t_ctype != C_CLERIC && player.t_ctype != C_PALADIN) {
	msg("Only clerics and paladins can affect undead.");
	return;
    }

    new_pos.y = hero.y + delta.y;
    new_pos.x = hero.x + delta.x;

    /* Anything there? */
    if (new_pos.y < 0 || new_pos.y > LINES-3 ||
	new_pos.x < 0 || new_pos.x > COLS-1 ||
	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
	msg("Nothing to affect.");
	return;
    }

    if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
	debug("Affect what @ %d,%d?", new_pos.y, new_pos.x);
	return;
    }
    tp = (struct thing *) ldata(item);
    mname = monsters[tp->t_index].m_name;

    if (off(*tp, ISUNDEAD) || on(*tp, WASTURNED)) goto annoy;

    /* Can cleric kill it? */
    if (pstats.s_lvl >= 3 * tp->t_stats.s_lvl) {
	msg("You have destroyed the %s.", mname);
	killed(&player, item, FALSE, TRUE);
	return;
    }

    /* Can cleric turn it? */
    if (rnd(100) + 1 >
	 (100 * ((2 * tp->t_stats.s_lvl) - pstats.s_lvl)) / pstats.s_lvl) {
	/* Make the monster flee */
	turn_on(*tp, WASTURNED);	/* No more fleeing after this */
	turn_on(*tp, ISFLEE);

	/* Let player know */
	msg("You have turned the %s.", mname);

	/* get points for turning monster */
	pstats.s_exp += tp->t_stats.s_exp/2;

	/* If monster was suffocating, stop it */
	if (on(*tp, DIDSUFFOCATE)) {
	    turn_off(*tp, DIDSUFFOCATE);
	    extinguish(suffocate);
	}

	/* If monster held us, stop it */
	if (on(*tp, DIDHOLD) && (--hold_count == 0))
		turn_off(player, ISHELD);
	turn_off(*tp, DIDHOLD);
	return;
    }

    /* Otherwise -- no go */
annoy:
    msg("You do not affect the %s.", mname);

    /* Annoy monster */
   if (off(*tp, WASTURNED)) runto(&new_pos, &hero);
}


/* Routines for magicians */


magician_cast()
{
    register short i, num_spells, which_spell, avail_points;
    register bool nohw = FALSE;

    /* Get the number of avilable spells */
    if (pstats.s_intel >= 16) 
	num_spells = pstats.s_intel - 15;
    else 
	num_spells = 0;

    if (player.t_ctype == C_MAGICIAN) 
	num_spells += pstats.s_lvl;

    if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC
 		&& player.t_ctype != C_ILLUSION && player.t_ctype != C_DRUID
		&& ISWEARING(R_WIZARD))
	num_spells *= 2;

    if (num_spells > MAXSPELLS_M) 
	num_spells = MAXSPELLS_M;

    avail_points = pstats.s_intel * pstats.s_lvl;
    if (ISWEARING(R_WIZARD))
	avail_points *= 2;

    /* Prompt for spells */
    msg(""); /* Kludge to rid us of irritating -- More -- */
    msg("Which spell are you casting [%d points left] (* for list)? ",
			avail_points - spell_power);

    which_spell = (short) ((getchar() & 0177) - 'a');
    if (which_spell == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which_spell >= 0 && which_spell < num_spells) 
	nohw = TRUE;
    else {
	register char c;
	char buf[LINELEN];
	char *buf2;

	for (i=0; i<num_spells; i++) {
	    sprintf(buf, "[%c, %d/%d] %s", 
		        (char) ((short) 'a' + i),
		        magic_spells[i].s_cost,
			avail_points - spell_power,
			(terse) ? "" : "A spell of ");
	    if (magic_spells[i].s_type == TYP_POTION)
		buf2 = p_magic[magic_spells[i].s_which].mi_name;
	    else if (magic_spells[i].s_type == TYP_SCROLL)
		buf2 = s_magic[magic_spells[i].s_which].mi_name;
	    else if (magic_spells[i].s_type == TYP_STICK)
		buf2 = ws_magic[magic_spells[i].s_which].mi_name;
	    strcat(buf, buf2);
	    if(num_spells == 1)
		msg(buf);
	    else
		add_line(buf);
	}
	if(num_spells != 1)
		end_line();
        msg(""); /* Kludge to rid us of irritating -- More -- */
	msg("Which spell are you casting? ");
    }

    if (!nohw) {
	which_spell = (short) ((getchar() & 0177) - 'a');
	while (which_spell < 0 || which_spell >= num_spells) {
	    if (which_spell == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("Please enter one of the listed spells: ");
	    which_spell = (short) ((getchar() & 0177) - 'a');
	}
    }

    if ((spell_power + magic_spells[which_spell].s_cost) > avail_points) {
	msg("Your attempt fails.");
	return;
    }

    msg("Your spell is successful.");
    if (magic_spells[which_spell].s_type == TYP_POTION)
        quaff(	magic_spells[which_spell].s_which,
        	magic_spells[which_spell].s_blessed);
    else if (magic_spells[which_spell].s_type == TYP_SCROLL)
        read_scroll(	magic_spells[which_spell].s_which,
        		magic_spells[which_spell].s_blessed);
    else if (magic_spells[which_spell].s_type == TYP_STICK) {
	 if (get_dir())
	      do_zap(	TRUE, 
			magic_spells[which_spell].s_which,
			magic_spells[which_spell].s_blessed);
	 else
	      return;
    }
    spell_power += magic_spells[which_spell].s_cost;
}


/*
 * Routines for illusionists
 */


illusion_cast()
{
    register short i, num_spells, which_spell, avail_points;
    register bool nohw = FALSE;

    /* Get the number of avilable spells */
    if (pstats.s_intel >= 16) 
	num_spells = pstats.s_intel - 15;
    else 
	num_spells = 0;

    if (player.t_ctype == C_ILLUSION) 
	num_spells += pstats.s_lvl;

    if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC
 		&& player.t_ctype != C_ILLUSION && player.t_ctype != C_DRUID
		&& ISWEARING(R_WIZARD))
	num_spells *= 2;

    if (num_spells > MAXSPELLS_I) 
	num_spells = MAXSPELLS_I;

    avail_points = pstats.s_intel * pstats.s_lvl;
    if (ISWEARING(R_WIZARD))
	avail_points *= 2;

    /* Prompt for spells */
    msg(""); /* Kludge to rid us of irritating -- More -- */
    msg("Which spell are you casting [%d points left] (* for list)? ",
			avail_points - spell_power);

    which_spell = (short) ((getchar() & 0177) - 'a');
    if (which_spell == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which_spell >= 0 && which_spell < num_spells) 
	nohw = TRUE;
    else {
	register char c;
	char buf[LINELEN];
	char *buf2;

	for (i=0; i<num_spells; i++) {
	    sprintf(buf, "[%c, %d/%d] %s", 
		        (char) ((short) 'a' + i),
		        illusion_spells[i].s_cost,
			avail_points - spell_power,
			(terse) ? "" : "A spell of ");
	    if (illusion_spells[i].s_type == TYP_POTION)
		buf2 = p_magic[illusion_spells[i].s_which].mi_name;
	    else if (illusion_spells[i].s_type == TYP_SCROLL)
		buf2 = s_magic[illusion_spells[i].s_which].mi_name;
	    else if (illusion_spells[i].s_type == TYP_STICK)
		buf2 = ws_magic[illusion_spells[i].s_which].mi_name;
	    strcat(buf, buf2);
	    if(num_spells == 1)
		msg(buf);
	    else
		add_line(buf);
	}
	if(num_spells != 1)
		end_line();
        msg(""); /* Kludge to rid us of irritating -- More -- */
	msg("Which spell are you casting? ");
    }

    if (!nohw) {
	which_spell = (short) ((getchar() & 0177) - 'a');
	while (which_spell < 0 || which_spell >= num_spells) {
	    if (which_spell == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("Please enter one of the listed spells: ");
	    which_spell = (short) ((getchar() & 0177) - 'a');
	}
    }

    if ((spell_power + illusion_spells[which_spell].s_cost) > avail_points) {
	msg("Your attempt fails.");
	return;
    }

    msg("Your spell is successful.");
    if (illusion_spells[which_spell].s_type == TYP_POTION)
        quaff(	illusion_spells[which_spell].s_which,
        	illusion_spells[which_spell].s_blessed);
    else if (illusion_spells[which_spell].s_type == TYP_SCROLL)
        read_scroll(	illusion_spells[which_spell].s_which,
        		illusion_spells[which_spell].s_blessed);
    else if (illusion_spells[which_spell].s_type == TYP_STICK) {
	 if (get_dir())
	      do_zap(	TRUE, 
			illusion_spells[which_spell].s_which,
			illusion_spells[which_spell].s_blessed);
	 else
	      return;
    }
    spell_power += illusion_spells[which_spell].s_cost;
}

/*
 * Last Modified: 05/25/86
 */
/*
 * File with various monster functions in it
 *
 */

#include "curses.h"
#include "rogue.h"
#include <ctype.h>
#include "mach_dep.h"

/*
 * summon_monster:
 *     Summon a monster.
 */
struct linked_list *
summon_monster(type,familiar,quiet)
short
    type; 
bool
    familiar,quiet;
{
register struct linked_list
    *mp;
register struct thing
    *tp;
short
    monster;

    if (type == 0) {	/* Summon a random monster modified by level */
    register int
	index;

	index = pstats.s_lvl + rnd(pstats.s_lvl);
	if (index >= 100)
	    index = 100 + rnd(25);

	monster = index;
    } else
	monster = type;

    turn_on(player, SUMMONING);
    if ( !(mp = creat_mons(&player, monster, FALSE))) {
        msg("Summon failed.");
	turn_off(player, SUMMONING);
	return(NULL);
    }
    else {
	if (!quiet) {
	    msg("A %s appears out of nowhere!",monsters[monster].m_name);
	    if (familiar)
	        msg("I am here to do your bidding master.");
	    else
		msg("Oh my goodness!  Are you Larry? ");
	}
	tp = THINGPTR(mp);
	turn_on(*tp,ISCHARMED); /* Summoned monsters are always charmed */
	if (familiar) {
	    if (familiar_ptr)
		killed(NULL, familiar_ptr, FALSE, FALSE);
	    turn_on(*tp, ISREGEN); 	/* Familiars can regenerate */
	    turn_on(*tp, ISSCAVENGE);   /* and pick up things */
	    turn_on(*tp, ISFAMILIAR);
	    familiar_ptr = mp;
	    familiar_type = monster;
	}
	turn_off(player, SUMMONING);
	return(mp);
    }
}

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */

short
randmonster(wander, no_summon)
register bool wander, no_summon;
{
    register int d, cur_level, range, i; 

    /* 
     * Do we want a merchant? Merchant is always in place 'NUMMONST' 
     */
    if (wander && monsters[NUMMONST].m_wander && rnd(100) < 3) return NUMMONST;

    cur_level = level;
    range = 4*NLEVMONS;
    i = 0;
    do
    {
	if (i++ > range*10) { /* just in case all have be genocided */
	    i = 0;
	    if (--cur_level <= 0)
		fatal("Rogue could not find a monster to make");
	}
	d = NLEVMONS*(cur_level - 1) + (rnd(range) - (range - 1 - NLEVMONS));
	if (d < 1)
	    d = rnd(NLEVMONS) + 1;
	if (d > NUMMONST - NUMSUMMON - 1) {
	    if (no_summon)
		d = rnd(range) + (NUMMONST - NUMSUMMON - 1) - (range - 1);
	    else if (d > NUMMONST - 1)
		d = rnd(range+NUMSUMMON) + (NUMMONST-1) - (range+NUMSUMMON-1);
	}
    }
    while  ( wander ? !monsters[d].m_wander || !monsters[d].m_normal 
		   : !monsters[d].m_normal);
    return d;
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */

new_monster(item, type, cp, max_monster)
struct linked_list *item;
short type;
register coord *cp;
bool max_monster;
{
    register struct thing *tp;
    register struct monster *mp;
    register char *ip, *hitp;
    register short i, min_intel, max_intel;
    register short num_dice, num_sides=8, num_extra=0;
    char *strchr();

    attach(mlist, item);
    tp = THINGPTR(item);
    tp->t_index = type;
    tp->t_wasshot = FALSE;
    tp->t_type = monsters[type].m_appear;
    tp->t_ctype = C_MONSTER;
    tp->t_no_move = 0;
    tp->t_doorgoal = -1;
    tp->t_pos = *cp;
    tp->t_oldpos = *cp;
    tp->t_oldch = mvwinch(cw, cp->y, cp->x);
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[tp->t_index];

    /* Figure out monster's hit points */
    hitp = mp->m_stats.s_hpt;
    num_dice = atoi(hitp);
    if ((hitp = strchr(hitp, 'd')) != NULL) {	/* strchr was index */
	num_sides = atoi(++hitp);
	if ((hitp = strchr(hitp, '+')) != NULL)
	    num_extra = atoi(++hitp);
    }

    if (max_monster)
	tp->t_stats.s_hpt = num_dice * num_sides + num_extra;
    else
	tp->t_stats.s_hpt = roll(num_dice, num_sides) + num_extra;
    tp->t_stats.s_lvl = mp->m_stats.s_lvl;
    tp->t_stats.s_arm = mp->m_stats.s_arm;
    tp->t_stats.s_dmg = mp->m_stats.s_dmg;
    tp->t_stats.s_exp = mp->m_stats.s_exp + mp->m_add_exp*tp->t_stats.s_hpt;
    tp->t_stats.s_str = mp->m_stats.s_str;

    if (max_level > 70) {
	tp->t_stats.s_hpt += roll(4,(max_level-60)*2);
	tp->t_stats.s_lvl += roll(4,(max_level-60)/8);
	tp->t_stats.s_arm -= roll(2,(max_level-60)/8);
	tp->t_stats.s_str += roll(2,(max_level-60)/12);
    }

    /*
     * just initailize others values to something reasonable for now
     * maybe someday will *really* put these in monster table
     */
    tp->t_stats.s_wisdom = 8 + rnd(4);
    tp->t_stats.s_dext = 8 + rnd(4);
    tp->t_stats.s_const = 8 + rnd(4);
    tp->t_stats.s_charisma = 8 + rnd(4);

    if (max_level > 60)
	tp->t_stats.s_dext += roll(2,(max_level-50)/8);

    /* Set the initial flags */
    for (i=0; i<16; i++) tp->t_flags[i] = 0;
    for (i=0; i<16; i++)
	turn_on(*tp, mp->m_flags[i]);

    /* suprising monsters don't always surprise you */
    if (!max_monster && on(*tp, CANSURPRISE) && rnd(100) < 20)
	    turn_off(*tp, CANSURPRISE);

    /* If this monster is unique, gen it */
    if (on(*tp, ISUNIQUE)) mp->m_normal = FALSE;

    tp->t_turn = TRUE;
    tp->t_pack = NULL;

    /* Normally scared monsters have a chance to not be scared */
    if (on(*tp, ISFLEE) && (rnd(4) == 0)) turn_off(*tp, ISFLEE);

    /* Figure intelligence */
    min_intel = atoi(mp->m_intel);
    if ((ip = (char *) strchr(mp->m_intel, '-')) == NULL)
	tp->t_stats.s_intel = min_intel;
    else {
	max_intel = atoi(++ip);
	if (max_monster)
	    tp->t_stats.s_intel = max_intel;
	else
	    tp->t_stats.s_intel = min_intel + rnd(max_intel - min_intel);
    }
    tp->maxstats = tp->t_stats;

    /* If the monster can shoot, it may have a weapon */
    if (on(*tp, CANSHOOT) && (rnd(9) < 6 || max_monster)) {
	struct linked_list *item, *item1;
	register struct object *cur, *cur1;

	item = new_item(sizeof *cur);
	item1 = new_item(sizeof *cur1);
	cur = OBJPTR(item);
	cur1 = OBJPTR(item1);
	cur->o_hplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur->o_dplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur1->o_hplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur1->o_dplus = (rnd(4) < 3) ? 0
				    : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
	cur->o_damage = cur->o_hurldmg =
		cur1->o_damage = cur1->o_hurldmg = "0d0";
	cur->o_ac = cur1->o_ac = 11;
	cur->o_count = cur1->o_count = 1;
	cur->o_group = cur1->o_group = 0;
	if ((cur->o_hplus <= 0) && (cur->o_dplus <= 0)) cur->o_flags = ISCURSED;
	if ((cur1->o_hplus <= 0) && (cur1->o_dplus <= 0))
	    cur1->o_flags = ISCURSED;
	cur->o_flags = cur1->o_flags = 0;
	cur->o_type = cur1->o_type = WEAPON;
	cur->o_mark[0] = cur1->o_mark[0] = '\0';

	/* The monster may use a crossbow, sling, footbow, or an arrow */
	i = rnd(100);
	if (i < 9) {
	    cur->o_which = CROSSBOW;
	    cur1->o_which = BOLT;
	    init_weapon(cur, CROSSBOW);
	    init_weapon(cur1, BOLT);
	}
	else if (i < 20) {
	    cur->o_which = FOOTBOW;
	    cur1->o_which = FBBOLT;
	    init_weapon(cur, FOOTBOW);
	    init_weapon(cur1, FBBOLT);
	}
	else if (i < 65) {
	    cur->o_which = BOW;
	    init_weapon(cur, BOW);
	    if (strcmp(mp->m_name,"elf") == 0 && rnd(10) < 1) {
		cur1->o_which = SILVERARROW;
		init_weapon(cur1, SILVERARROW);
	    }
	    else {
		cur1->o_which = ARROW;
		init_weapon(cur1, ARROW);
            }
	}
	else {
	    cur->o_which = SLING;
	    cur1->o_which = ROCK;
	    init_weapon(cur, SLING);
	    init_weapon(cur1, ROCK);
	}

	attach(tp->t_pack, item);
	attach(tp->t_pack, item1);
    }


    if (ISWEARING(R_AGGR))
	runto(cp, &hero);
    if (on(*tp, ISDISGUISE))
    {
	char mch;

	if (tp->t_pack != NULL)
	    mch = (OBJPTR(tp->t_pack))->o_type;
	else
	    switch (rnd(level > arts[0].ar_level ? 10 : 9))
	    {
		when 0: mch = GOLD;
		when 1: mch = POTION;
		when 2: mch = SCROLL;
		when 3: mch = FOOD;
		when 4: mch = WEAPON;
		when 5: mch = ARMOR;
		when 6: mch = RING;
		when 7: mch = STICK;
		when 8: mch = monsters[randmonster(FALSE, FALSE)].m_appear;
		when 9: mch = ARTIFACT;
	    }
	tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */

wanderer()
{
    register int i, count = 0;
    register struct room *hr = roomin(&hero);
    register struct linked_list *item;
    register struct thing *tp;
    coord cp;
    char *loc;
    int which;

    /* Find a place for it -- avoid the player's room */
    do {
	do {
	    count++;
	    i = rnd_room();
	} until(hr != &rooms[i] || levtype == MAZELEV 
			|| levtype == THRONE || count > 500);
	rnd_pos(&rooms[i], &cp);
    } until (step_ok(cp.y, cp.x, NOMONST, (struct thing *) NULL));

    /* Create a new wandering monster */
    item = new_item(sizeof *tp);
    which = randmonster(TRUE, FALSE);
    new_monster(item, which, &cp, FALSE);
    tp = THINGPTR(item);
    turn_on(*tp, ISRUN);
    turn_off(*tp, ISDISGUISE);
    tp->t_dest = &hero;

    tp->t_pos = cp;	/* Assign the position to the monster */

    i = rnd(7);
    if (on(*tp, ISSWARM) && i < 5)
	count = roll(2,4);
    else if (on(*tp, ISFLOCK) && i < 5)
	count = roll(1,4);
    else
	count = 0;

    for (i = 1; i <= count; i++)
	creat_mons(tp, which, FALSE);

    i = DISTANCE(cp.x, cp.y, hero.x, hero.y);

    if (i < 20)
	loc = "very close to you";
    else if (i < 400)
	loc = "nearby";
    else
	loc = "in the distance";

    if (on(player, CANSCENT) || (player.t_ctype == C_THIEF && rnd(40) == 0)) 
	msg("You smell a new %s %s.", monsters[tp->t_index].m_name, loc);
    if (on(player, CANHEAR) || (player.t_ctype == C_THIEF && rnd(40) == 0)) 
	msg("You hear a new %s moving %s.", monsters[tp->t_index].m_name, loc);

    debug("Started a wandering %s.", monsters[tp->t_index].m_name);
}

/*
 * what to do when the hero steps next to a monster
 */
struct linked_list *
wake_monster(y, x)
int y, x;
{
    register struct thing *tp;
    register struct linked_list *it;
    register struct room *rp, *trp;
    register char ch;
    register char *mname;

    if ((it = find_mons(y, x)) == NULL) {
	debug("Can't find monster in show.");
	return(NULL);
    }
    tp = THINGPTR(it);
    ch = tp->t_type;

    if ((good_monster(*tp)) || on(player, SUMMONING)) {
	tp->t_dest = &hero;
	turn_on(*tp, ISRUN);
	turn_off(*tp, ISDISGUISE);
	return(it);
    }

    rp = roomin(&hero);	/* Get the current room for hero */
    trp = roomin(&tp->t_pos); /* Current room for monster */
    mname = monsters[tp->t_index].m_name;


    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && off(*tp, ISRUN))
	if ((trp != NULL) && (lvl_obj != NULL)) {
	    struct linked_list *item;
	    struct object *cur;

	    for (item = lvl_obj; item != NULL; item = next(item)) {
		cur = OBJPTR(item);
		if ((cur->o_type == GOLD) &&
		    (roomin(&cur->o_pos) == trp)) {
		    /* Run to the gold */
		    tp->t_dest = &cur->o_pos;
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);

		    /* Make it worth protecting */
		    cur->o_count += roll(2,3) * GOLDCALC;
		    break;
		}
	    }
	}

    /*
     * Every time he sees mean monster, it might start chasing him
     * unique monsters always do
     */
    if (on(*tp, ISUNIQUE) || rnd(100) > 33 && on(*tp, ISMEAN) && 
	off(*tp, ISHELD) && off(*tp, ISRUN) && !is_stealth(&player) &&
	(off(player, ISINVIS) || on(*tp, CANSEE)))
    {
	tp->t_dest = &hero;
	turn_on(*tp, ISRUN);
	turn_off(*tp, ISDISGUISE);
    }

    /* Handle monsters that can gaze */
    if (on(*tp, ISRUN) && 	/* Monster is not asleep */
	cansee(tp->t_pos.y, tp->t_pos.x)) {	/* Player can see monster */
	/*
	 * Confusion
	 */
	if (on(*tp, CANHUH)) {
	    if (on(player, CANREFLECT)) {
		msg("You reflect the gaze of the %s.", mname);
		msg("The %s is confused !!!", mname);
		turn_on(*tp, ISHUH);
	    } else 
	    if (!save(VS_MAGIC)) {
		if (off(player, ISCLEAR) && off(player, ISINVIS)) {
		    if (on(player, ISHUH))
			lengthen(unconfuse, rnd(20)+HUHDURATION);
		    else {
			fuse(unconfuse, 0, rnd(20)+HUHDURATION, AFTER);
			msg("The %s's gaze has confused you.",mname);
			turn_on(player, ISHUH);
		    }
		}
		else if (on(player, ISINVIS))
		    msg("The %s's gaze has no effect.", mname);
		else 
		    msg("You feel dizzy for a moment, but it quickly passes.");
	    }
	    else if (rnd(100) < 67)
		turn_off(*tp, CANHUH); /* Once you save, maybe that's it */
	}

	/* Sleep */
        if (on(*tp, CANSNORE)) 

	if (on(player, CANREFLECT)) {
	    msg("You reflect the gaze of the %s", mname);
	    msg("The %s falls asleep !!!", mname);
	    tp->t_no_move += SLEEPTIME;
	} else
	if (no_command == 0 && !save(VS_PARALYZATION)) {
 	    if (on(player, ISINVIS))
		msg("The gaze of the %s has no effect.", mname);
	    else if (ISWEARING(R_ALERT))
		msg("You feel slightly drowsy for a moment.");
	    else {
		msg("The %s's gaze puts you to sleep.", mname);
		no_command = SLEEPTIME;
		if (rnd(100) < 50) turn_off(*tp, CANSNORE);
	    }
	}

	/* Fear */
	if (on(*tp, CANFRIGHTEN)) {
	    if (on(player, CANREFLECT)) {
		msg("The %s looks at you and sees its reflection. ", mname);
		msg("The %s is terrified by its reflection !!!", mname);
		turn_on(*tp, ISFLEE);
	    } else {
	        turn_off(*tp, CANFRIGHTEN);
	        if (!save(VS_WAND) &&
	            !(on(player, ISFLEE) && (player.t_dest == &tp->t_pos))) {
		        if (off(player, SUPERHERO) && off(player, ISINVIS)) {
			    turn_on(player, ISFLEE);
			    player.t_dest = &tp->t_pos;
			    msg("The sight of the %s terrifies you.", mname);
		        }
		        else
			    msg("My, the %s looks ugly.", mname);
	        }
	    }
	}

	/*
	 * slow
	 */
	if (on(*tp, LOOKSLOW)) {
	    if (on(player, CANREFLECT)) {
		msg("You reflect the gaze of the %s.", mname);
		msg("The %s is slowing down !!!", mname);
		turn_on(*tp, ISSLOW);
	    } 
	    else
	    if (!save(VS_MAGIC)) {
		if (on(player, ISHASTE)) { /* Already sped up */
		    extinguish(nohaste);
		    nohaste();
		}
		else {
		    msg("You feel yourself moving %sslower.",
			    on(player, ISSLOW) ? "even " : "");
		    if (on(player, ISSLOW))
			lengthen(noslow, rnd(4) + 4);
		    else if (!ISWEARING(R_FREEDOM)) {
			turn_on(player, ISSLOW);
			player.t_turn = TRUE;
			fuse(noslow, 0, rnd(4)+4, AFTER);
		    }
		}
	    }
	    turn_off(*tp, LOOKSLOW); 
	}

	/* blinding creatures */
	if (on(*tp, CANBLIND)) 
	if (on(player, CANREFLECT)) {
	    msg("You reflect the gaze of the %s.", mname);
	    msg("The %s is blinded !!!", mname);
	    turn_on(*tp, ISHUH);
	} else
	if (off(player, ISBLIND) && !ISWEARING(R_SEEINVIS)
		&& off(player, ISINVIS)
		&& !save(VS_WAND)) {
	    msg("The gaze of the %s blinds you.", mname);
	    turn_on(player, ISBLIND);
	    fuse(sight, 0, rnd(30)+20, AFTER);
	    look(FALSE);
	}

	/* Turning to stone */
	if (on(*tp, LOOKSTONE)) 
	if (on(player, CANREFLECT)) {
	    msg("You reflect the gaze of the %s.", mname);
	    msg("The %s is turned to stone !!!", mname);
	    killed(player, it, FALSE, TRUE);
        } else {

	    turn_off(*tp, LOOKSTONE);

	    if (on(player, CANINWALL) || on(player, ISINVIS) || 
		on(player, CANREFLECT))
		msg("The gaze of the %s has no effect.", mname);
	    else {
		if (!save(VS_PETRIFICATION) && rnd(100) < 3) {
		    msg("The gaze of the %s petrifies you.", mname);
		    msg("You are turned to stone !!! --More--");
		    wait_for(' ');
		    death(D_PETRIFY);
		    return it;
		}
		else {
		    msg("The gaze of the %s stiffens your limbs.", mname);
		    no_command = STONETIME;
		}
	    }
	}
    }

    /*
     * Hide invisible monsters and those in a wall
     */
    if (((on(*tp, ISINVIS) || (on(*tp, ISSHADOW) && rnd(100) < 90)) &&
	off(player, CANSEE)) ||
	on(*tp, ISINWALL) ||
	on(*tp, CANSURPRISE))
	ch = mvwinch(stdscr, y, x);

    /*
     * hero might be able to hear or smell monster if he can't see it
     */
    if ((rnd(player.t_ctype == C_THIEF ? 40 : 200) == 0 || on(player, CANHEAR))
		 && !cansee(tp->t_pos.y, tp->t_pos.x))
	msg("You hear a %s nearby.", mname);
    if ((rnd(player.t_ctype == C_THIEF ? 40 : 200) == 0 || on(player, CANSCENT))
		 && !cansee(tp->t_pos.y, tp->t_pos.x))
	msg("You smell a %s nearby.", mname);

    return it;
}

genocide()
{
    register struct linked_list *ip;
    register struct thing *mp;
    register int i;
    register struct linked_list *nip;
    register int pres_monst=1, num_lines=LINES-3;
    register short which_monst;
    char monst_name[LINELEN*2];

    /* Print out the monsters */
    while (pres_monst < NUMMONST) {
	wclear(hw);
	touchwin(hw);

	wmove(hw, 2, 0);
	for (i=0; i<num_lines && pres_monst < NUMMONST; i++) {
	    sprintf(monst_name, "[%d] %s\n",
				pres_monst, monsters[pres_monst].m_name);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	if (pres_monst < NUMMONST) {
	    mvwaddstr(hw, LINES-1, 0, morestr);
	    draw(hw);
	    wait_for(' ');
	}

	else {
	    mvwaddstr(hw, 0, 0, "Which monster");
	    if (!terse) waddstr(hw, " do you wish to wipe out");
	    waddstr(hw, "? ");
	    draw(hw);
	}
    }

get_monst:
    get_str(monst_name, hw);
    which_monst = atoi(monst_name);
    if ((which_monst < 1 || which_monst > NUMMONST-1)) {
	mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range -- ");
	draw(hw);
	goto get_monst;
    }

    /* Set up for redraw */
    clearok(cw, TRUE);
    touchwin(cw);

    /* Remove this monster from the present level */
    for (ip = mlist; ip; ip = nip) {
	mp = THINGPTR(ip);
	nip = next(ip);
	if (mp->t_index == which_monst) {
	    check_residue(mp);	/* Check for special features before removing */
	    remove(&mp->t_pos, ip);
	}
    }

    /* Remove from available monsters */
    monsters[which_monst].m_normal = FALSE;
    monsters[which_monst].m_wander = FALSE;
    mpos = 0;
    msg("You have wiped out the %s.", monsters[which_monst].m_name);
}


/*
 * id_monst lists the monsters with the displayed by the character
 *          unless there is only one in which case it is returned
 *          as the string
 */

void
id_monst(monster)
register char monster;
{
    register int i;

    for (i = 1; i <= NUMMONST+2; i++)
	if (monsters[i].m_appear == monster)
	    add_line("A %s ", monsters[i].m_name);
    end_line();
}


/*
 * Check_residue takes care of any effect of the monster 
 */
check_residue(tp)
register struct thing *tp;
{
    /*
     * Take care of special abilities
     */
    if (on(*tp, DIDHOLD) && (--hold_count == 0)) turn_off(player, ISHELD);

    /* If it has lowered player, give him back a level, maybe */
    if (on(*tp, DIDDRAIN) && rnd(3) == 0) raise_level();

    /* If frightened of this monster, stop */
    if (on(player, ISFLEE) &&
	player.t_dest == &tp->t_pos) turn_off(player, ISFLEE);

    /* If monster was suffocating player, stop it */
    if (on(*tp, DIDSUFFOCATE)) extinguish(suffocate);

    /* If something with fire, may darken */
    if (on(*tp, HASFIRE)) {
	register struct room *rp=roomin(&tp->t_pos);

	if (rp && (--(rp->r_fires) <= 0)) {
	    rp->r_flags &= ~HASFIRE;
	    light(&tp->t_pos);
	}
    }
}

/* Sell displays a menu of goods from which the player may choose
 * to purchase something.
 */

sell(tp)
register struct thing *tp;
{
    register struct linked_list *item;
    register int i, j, min_worth, nitems, goods, chance, which_item, w;
    register struct object *obj;
    char buffer[LINELEN];
    char dbuf[LINELEN];
    struct {
	int which;
	int plus1, plus2;
	int count;
	int worth;
	int flags;
	char *name;
    } selection[10];

    min_worth = -1; 		/* hope item is never worth less than this */
    item = find_mons(tp->t_pos.y, tp->t_pos.x); /* Get pointer to monster */

    /* Select the items */
    nitems = rnd(6) + 5;
    switch (rnd(6)) {
	/* Armor */
	case 0:
	case 1:
	    goods = ARMOR;
	    for (i=0; i<nitems; i++) {
		chance = rnd(100);
		for (j = 0; j < MAXARMORS; j++)
		    if (chance < armors[j].a_prob)
			break;
		if (j == MAXARMORS) {
		    debug("Picked a bad armor %d", chance);
		    j = 0;
		}
		selection[i].which = j;
		selection[i].count = 1;
		if (rnd(100) < 40) selection[i].plus1 = rnd(5) + 1;
		else selection[i].plus1 = 0;
		selection[i].name = armors[j].a_name;
		switch(luck) {
		    when 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(5);
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(5);
			}
	        }

		/* Calculate price */
		w = armors[j].a_worth;
		w *= (1 + luck + (10 * selection[i].plus1));
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if (min_worth > selection[i].worth || i == 1)
		    min_worth = selection[i].worth;
	    }
	    break;

	/* Weapon */
	case 2:
	case 3:
	    goods = WEAPON;
	    for (i=0; i<nitems; i++) {
		selection[i].which = rnd(MAXWEAPONS);
		selection[i].count = 1;
		if (rnd(100) < 35) {
		    selection[i].plus1 = rnd(3);
		    selection[i].plus2 = rnd(3);
		}
		else {
		    selection[i].plus1 = 0;
		    selection[i].plus2 = 0;
		}
		if (weaps[selection[i].which].w_flags & ISMANY)
		    selection[i].count = rnd(15) + 8;
		selection[i].name = weaps[selection[i].which].w_name;
		switch(luck) {
		    when 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -rnd(3);
			    selection[i].plus2 = -rnd(3);
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -rnd(3);
			    selection[i].plus2 = -rnd(3);
			}
	        }
		w = weaps[selection[i].which].w_worth * selection[i].count;
		w *= (1 + luck + (10 * selection[i].plus1 + 
			10 * selection[i].plus2));
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if (min_worth > selection[i].worth || i == 1)
		    min_worth = selection[i].worth;
	    }
	    break;

	/* Staff or wand */
	case 4:
	    goods = STICK;
	    for (i=0; i<nitems; i++) {
		selection[i].which = pick_one(ws_magic, MAXSTICKS);
		selection[i].plus1 = rnd(11) + 5;	/* Charges */
		selection[i].count = 1;
		selection[i].name = ws_magic[selection[i].which].mi_name;
		switch(luck) {
		    when 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = 1;
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = 1;
			}
	        }
		w = ws_magic[selection[i].which].mi_worth;
		w += (luck + 1) * 20 * selection[i].plus1;
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if (min_worth > selection[i].worth || i == 1)
		    min_worth = selection[i].worth;
	    }
	    break;

	/* Ring */
	case 5:
	    goods = RING;
	    for (i=0; i<nitems; i++) {
		selection[i].which = pick_one(r_magic, MAXRINGS);
		selection[i].plus1 = rnd(2) + 1;  /* Armor class */
		selection[i].count = 1;
		if (rnd(100) < r_magic[selection[i].which].mi_bless + 10)
		    selection[i].plus1 += rnd(2) + 1;
		selection[i].name = r_magic[selection[i].which].mi_name;
		switch(luck) {
		    when 0: break;
		    when 1:
			if (rnd(3) == 0) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(2);
			}
		    otherwise:
			if (rnd(luck)) {
			    selection[i].flags |= ISCURSED;
			    selection[i].plus1 = -1 - rnd(2);
			}
	        }
		w = r_magic[selection[i].which].mi_worth;
		switch (selection[i].which) {
		case R_DIGEST:
		    if (selection[i].plus1 > 2) selection[i].plus1 = 2;
		    else if (selection[i].plus1 < 1) selection[i].plus1 = 1;
		/* fall thru here to other cases */
		case R_ADDSTR:
		case R_ADDDAM:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDINTEL:
		case R_ADDWISDOM:
		    if (selection[i].plus1 > 0)
			w += selection[i].plus1 * 50;
		}
		w *= (1 + luck);
		w = (w/2) + (roll(6,w)/6);
		selection[i].worth = max(w, 25);
		if(min_worth > selection[i].worth * selection[i].count)
		    min_worth = selection[i].worth;
	    }
    }

    /* See if player can afford an item */
    if (min_worth > purse) {
	msg("The %s eyes your small purse and departs.",
			monsters[NUMMONST].m_name);
	/* Get rid of the monster */
	killed(NULL, item, FALSE, FALSE);
	return;
    }

    /* Display the goods */
    msg("The %s shows you his wares.", monsters[NUMMONST].m_name);
    wstandout(cw);
    mvwaddstr(cw, 0, mpos, morestr);
    wstandend(cw);
    draw(cw);
    wait_for(' ');
    msg("");
    clearok(cw, TRUE);
    touchwin(cw);

    wclear(hw);
    touchwin(hw);
    for (i=0; i < nitems; i++) {
	if (selection[i].worth > purse)
	    continue;
	wmove(hw, i+2, 0);
	sprintf(dbuf,"[%c] ", (char) ((short) 'a' + i));
	switch (goods) {
	    when ARMOR:
		strcat(dbuf, "Some ");
	    when WEAPON:
		if (selection[i].count == 1)
		    strcat(dbuf, "A ");
		else {
		    sprintf(buffer, "%2d ", selection[i].count);
		    strcat(dbuf, buffer);
		}
	    when STICK:
		strcat(dbuf, "A ");
		strcat(dbuf, ws_type[selection[i].which]);
		strcat(dbuf, " of ");
	    when RING:
		strcat(dbuf, "A ring of ");
	}
	strcat(dbuf, selection[i].name);
	if (selection[i].count > 1)
	    strcat(dbuf, "s");
	sprintf(buffer, "%-50s Price:  %d", dbuf, selection[i].worth);
	waddstr(hw, buffer);
    }
    sprintf(buffer, "Purse:  %d", purse);
    mvwaddstr(hw, nitems+3, 0, buffer);
    mvwaddstr(hw, 0, 0, "How about one of the following goods? ");
    draw(hw);
    /* Get rid of the monster */
    killed(NULL, item, FALSE, FALSE);

    which_item = (short) ((getchar() & 0177) - 'a');
    while (which_item < 0 || which_item >= nitems) {
	if (which_item == (short) ESCAPE - (short) 'a') {
	    return;
	}
	mvwaddstr(hw, 0, 0, "Please enter one of the listed items: ");
	draw(hw);
	which_item = (short) ((getchar() & 0177) - 'a');
    }

    purse -= selection[which_item].worth;

    item = spec_item(goods, selection[which_item].which,
		     selection[which_item].plus1, selection[which_item].plus2);

    obj = OBJPTR(item);
    if (selection[which_item].count > 1) {
	obj->o_count = selection[which_item].count;
	obj->o_group = newgrp();
    }

    if (selection[i].flags) {
	obj->o_flags |= ISCURSED;
	obj->o_flags &= ~ISBLESSED;
    }

    /* If a stick or ring, let player know the type */
    switch (goods) {
	when STICK: ws_know[selection[which_item].which] = TRUE;
	when RING:  r_know[selection[which_item].which] = TRUE;
    }

    if (add_pack(item, FALSE) == FALSE) {
	obj->o_pos = hero;
	fall(&player, item, TRUE);
    }
}

/*
 * Last Modified: 05/25/86
 */
/*
 * Functions to implement the various sticks one might find
 * while wandering around the dungeon.
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

fix_stick(cur)
register struct object *cur;
{
    if (strcmp(ws_type[cur->o_which], "staff") == 0) {
	cur->o_weight = 100;
	cur->o_charges = 5 + rnd(10);
	cur->o_damage = "2d3";
	switch (cur->o_which) {
	    when WS_HIT:
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		cur->o_damage = "2d8";
	    when WS_LIGHT:
		cur->o_charges = 20 + rnd(10);
	    }
    }
    else {
	cur->o_damage = "1d3";
	cur->o_weight = 60;
	cur->o_charges = 3 + rnd(5);
	switch (cur->o_which) {
	    when WS_HIT:
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		cur->o_damage = "1d8";
	    when WS_LIGHT:
		cur->o_charges = 10 + rnd(10);
	    }
    }
    cur->o_hurldmg = "1d1";

}

do_zap(gotdir, which, blessed)
bool gotdir;
int which;
bool blessed;
{
    register struct linked_list *item, *nitem;
    register struct object *obj, *nobj;
    register struct room *rp;
    register struct thing *tp;
    register int y, x;
    bool cursed, is_stick;

    cursed = FALSE;
    is_stick = FALSE;

    if (which == NULL) {
	if ((item = get_item("zap with", STICK)) == NULL)
	    return;
	obj = OBJPTR(item);
	if (obj->o_type != STICK && !(obj->o_flags & ISZAPPED)) {
	    msg("You can't zap with that!");
	    return;
	}
	if (obj->o_type != STICK)
	    which = WS_ELECT;
	else
	    which = obj->o_which;
	ws_know[which] = TRUE;
	if (obj->o_charges < 1)
	{
	    msg("Nothing happens.");
	    return;
	}
	obj->o_charges--;
	if (!gotdir)
	    do {
		delta.y = rnd(3) - 1;
		delta.x = rnd(3) - 1;
	    } while (delta.y == 0 && delta.x == 0);

	cursed = obj->o_flags & ISCURSED;
	blessed = obj->o_flags & ISBLESSED;
	is_stick = TRUE;
    }
    switch (which)
    {
	when WS_LIGHT:
	    /*
	     * Reddy Kilowat wand.  Light up the room
	     */
	    blue_light(blessed, cursed);
	when WS_DRAIN:
	    /*
	     * Take away 1/2 of hero's hit points, then take it away
	     * evenly from the monsters in the room or next to hero
	     * if he is in a passage (but leave the monsters alone
	     * if the stick is cursed)
	     * Drain no points from hero if blessed
	     */
	    if (pstats.s_hpt < 2) {
		msg("You are too weak to use it.");
		return;
	    }
	    if (cursed)
		pstats.s_hpt /= 2;
	    else if ((rp = roomin(&hero)) == NULL)
		drain(hero.y-1, hero.y+1, hero.x-1, hero.x+1);
	    else
		drain(rp->r_pos.y, rp->r_pos.y+rp->r_max.y,
		    rp->r_pos.x, rp->r_pos.x+rp->r_max.x);
	    if (blessed)
		pstats.s_hpt *= 2.0/3.0;
	when WS_POLYMORPH:
	case WS_TELMON:
	case WS_CANCEL:
	{
	    register char monster, oldch;
	    register int rm;
	    register int save_adj = 0;

	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(monster = mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		/* if the monster gets the saving throw, leave the case */
		if (blessed) save_adj = -5;
		if (cursed)
		    if (which == WS_POLYMORPH)
			save_adj = -5;	/* not save vs becoming tougher */
		    else save_adj = 5;

		if (save_throw(VS_MAGIC - save_adj, tp)) {
		    msg("Nothing happens.");
		    break;
		}

		/* Unhold player */
		if (on(*tp, DIDHOLD)) {
		    turn_off(*tp, DIDHOLD);
		    if (--hold_count == 0) turn_off(player, ISHELD);
		}
		/* unsuffocate player */
		if (on(*tp, DIDSUFFOCATE)) {
		    turn_off(*tp, DIDSUFFOCATE);
		    extinguish(suffocate);
		}

		if (which == WS_POLYMORPH) {
		    register int which_new;

		    detach(mlist, item);
		    oldch = tp->t_oldch;
		    delta.y = y;
		    delta.x = x;
		    if (!blessed && !cursed)
			which_new = randmonster(TRUE, TRUE);
		    else {
			/* duplicate randmonster() for now */
			/* Eventually fix to take level */
			register int cur_level, range, i; 
			if (blessed) cur_level = level/2;
			if (cursed) cur_level = level*2;
			range = 4*NLEVMONS;
			i = 0;
			do
			{
			    if (i++ > range*10) { /* just in case all have be genocided */
				i = 0;
				if (--cur_level <= 0)
				    fatal("Rogue could not find a monster to make");
			    }
			    which_new = NLEVMONS*(cur_level - 1) + (rnd(range) - (range - 1 - NLEVMONS));
			    if (which_new < 1)
				which_new = rnd(NLEVMONS) + 1;
			    if (which_new > NUMMONST - NUMSUMMON - 1) {
				if (blessed)
				    which_new = rnd(range) + (NUMMONST - NUMSUMMON - 1) - (range - 1);
				else if (which_new > NUMMONST - 1)
				    which_new = rnd(range+NUMSUMMON) + (NUMMONST-1) - (range+NUMSUMMON-1);
			    }
			}
			while (!monsters[which_new].m_normal);
		    }
		    new_monster(item, which_new, &delta, FALSE);
		    monster = tp->t_type;
		    if (off(*tp, ISRUN))
			runto(&delta, &hero);
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, monster);
		    tp->t_oldch = oldch;
		    msg(terse ? "A new %s!" : "You have created a new %s!",
			monsters[tp->t_index].m_name);
		}
		else if (which == WS_CANCEL) {
		    tp->t_flags[0] &= CANC0MASK;
		    tp->t_flags[1] &= CANC1MASK;
		    tp->t_flags[2] &= CANC2MASK;
		    tp->t_flags[3] &= CANC3MASK;
		    tp->t_flags[4] &= CANC4MASK;
		    tp->t_flags[5] &= CANC5MASK;
		    tp->t_flags[6] &= CANC5MASK;
		    tp->t_flags[7] &= CANC7MASK;
		    tp->t_flags[8] &= CANC8MASK;
		    tp->t_flags[9] &= CANC9MASK;
		    tp->t_flags[10] &= CANCAMASK;
		    tp->t_flags[11] &= CANCBMASK;
		    tp->t_flags[12] &= CANCCMASK;
		    tp->t_flags[13] &= CANCDMASK;
		    tp->t_flags[14] &= CANCEMASK;
		    tp->t_flags[15] &= CANCFMASK;
		}
		else { /* A teleport stick */
		    if (cursed) {	/* Teleport monster to player */
			if ((y == (hero.y + delta.y)) &&
			    (x == (hero.x + delta.x)))
				msg("Nothing happens.");
			else {
			    tp->t_pos.y = hero.y + delta.y;
			    tp->t_pos.x = hero.x + delta.x;
			}
		    }
		    else if (blessed) {	/* Get rid of monster */
			killed(NULL, item, FALSE, FALSE);
			return;
		    }
		    else {
			register int i=0;

			do {	/* Move monster to another room */
			    rm = rnd_room();
			    rnd_pos(&rooms[rm], &tp->t_pos);
			} until(winat(tp->t_pos.y,tp->t_pos.x) ==
			        FLOOR || i++>500);
		    }

		    /* Now move the monster */
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, tp->t_oldch);
		    tp->t_dest = &hero;
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);
		    mvwaddch(mw, y, x, ' ');
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, monster);
		    if (tp->t_pos.y != y || tp->t_pos.x != x)
			tp->t_oldch = mvwinch(cw, tp->t_pos.y, tp->t_pos.x);
		}
	    }
	}
	when WS_MISSILE:
	{
	    static struct object bolt =
	    {
		'*' , {0, 0}, "", 0, 0, "1d4" , 0, 0, 100, 1
	    };

	    struct linked_list *item;
	    struct thing *tp;
	    int damage;
	    int nsides = 4;
	    char ch;
	    char *mname;

	    /* Magic Missiles *always* hit, no saving throw */
	    do_motion(&bolt, delta.y, delta.x, &player);
	    ch = winat(bolt.o_pos.y, bolt.o_pos.x);

	    if (cursed)
		nsides /= 2;
	    else if (blessed)
		nsides *= 2;
	    damage = roll(pstats.s_lvl, nsides);

	    if (isalpha(ch)) {
		item = find_mons(bolt.o_pos.y, bolt.o_pos.x);
		tp = THINGPTR(item);
		mname = monsters[tp->t_index].m_name;

		debug("Missiled %s for %d (%d)",
		    mname, damage, tp->t_stats.s_hpt-damage);
		if ((tp->t_stats.s_hpt -= damage) <= 0) {
		    msg("The missile kills the %s.", 
			on(player, ISBLIND) ? "monster" : mname);
		    killed(&player, item, FALSE, TRUE);
		}
		else {
		    msg("The missile hits the %s", mname);
		    runto(&bolt.o_pos, &hero);
		    summon(tp);
		}
	    }
	    if ( bolt.o_pos.y >= 0 && bolt.o_pos.x >= 0 &&
			    bolt.o_pos.y < LINES && bolt.o_pos.x < COLS)
		mvwaddch(cw, bolt.o_pos.y, bolt.o_pos.x,
			show(bolt.o_pos.y, bolt.o_pos.x));
	}
	when WS_HIT:
	{
	    register char ch;
	    struct object strike; /* don't want to change sticks attributes */

	    delta.y += hero.y;
	    delta.x += hero.x;
	    ch = winat(delta.y, delta.x);
	    if (isalpha(ch))
	    {
		static char buf[20];
		register int nsides, ndice;

		strike = *obj;
		if (blessed)	 strike.o_hplus  = 12;
		else if (cursed) strike.o_hplus  = 3;
		else		 strike.o_hplus  = 6;

		if (strcmp(ws_type[which], "staff") == 0)
		     ndice = 3;
		else ndice = 2;
		if (blessed)	 nsides  = 16;
		else if (cursed) nsides  = 4;
		else		 nsides  = 8;
		sprintf(buf, "%dd%d", ndice, nsides);

		strike.o_damage = buf;
		fight(&delta, ch, &strike, FALSE);
	    }
	}
	case WS_SLOW_M:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (cursed)
		{
		    if (on(*tp, ISSLOW))
			turn_off(*tp, ISSLOW);
		    else
			turn_on(*tp, ISHASTE);
		}
		else if (blessed) {
		    turn_off(*tp, ISRUN);
		    turn_on(*tp, ISHELD);
		    return;
		}
		else {
		    if (on(*tp, ISHASTE))
			turn_off(*tp, ISHASTE);
		    else
			turn_on(*tp, ISSLOW);
		    tp->t_turn = TRUE;
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	when WS_CHARGE:
	    if (ws_know[WS_CHARGE] != TRUE && is_stick)
		msg("This is a wand of charging.");
	    if ((nitem = get_item("charge", STICK)) != NULL) {
		nobj = OBJPTR(nitem);
		if ( !cursed && (nobj->o_charges == 0) && (nobj->o_which == WS_HIT))
		    fix_stick(nobj);
		else
		if (blessed)	 nobj->o_charges += 2;
		else if (cursed) nobj->o_charges -= 1;
		else		 nobj->o_charges += 1;
	    }
	when WS_ELECT:
	case WS_FIRE:
	case WS_COLD:
	    {
		char *name;
		int damage = roll(6,6) + BOLT_LENGTH;

		switch (which) {
		when WS_ELECT:
		    name = "lightning bolt";
		    if (obj->o_charges > 50)
			damage = roll(6+rnd(6),6+rnd(3)) + BOLT_LENGTH;
		when WS_FIRE:
		    name = "flame";
		when WS_COLD:
		    name = "ice";
		}
		shoot_bolt(&player, hero, delta, TRUE, D_BOLT, name, damage);
	    }
	when WS_ANTIM: {
	    reg int m1, m2, x1, y1;
	    reg char ch;
	    reg struct linked_list *ll;
	    reg struct thing *lt;

	    y1 = hero.y;
	    x1 = hero.x;
	    do {
		y1 += delta.y;
		x1 += delta.x;
		ch = winat(y1,x1);
	    } while (ch == PASSAGE || ch == FLOOR);
	    for (m1 = x1 - 1 ; m1 <= x1 + 1 ; m1++) {
		for(m2 = y1 - 1 ; m2 <= y1 + 1 ; m2++) {
		    ch = winat(m2,m1);
		    if (m1 == hero.x && m2 == hero.y)
			continue;
		    if (ch != ' ') {
			ll = find_obj(m2,m1);
			if (ll != NULL) {
			    detach(lvl_obj,ll);
			    discard(ll);
			}
			ll = find_mons(m2,m1);
			if (ll != NULL) {
			    lt = THINGPTR(ll);
			    if (on(*lt, CANSELL)) {
				luck++;
				aggravate();
			    }
			    if (off(*lt, CANINWALL)) {
				check_residue(lt);
				detach(mlist,ll);
				discard(ll);
				mvwaddch(mw,m2,m1,' ');
			    }
			}
			mvaddch(m2,m1,' ');
			mvwaddch(cw,m2,m1,' ');
		    }
		}
	    }
	    touchwin(cw);
	    touchwin(mw);
	}
	when WS_CONFMON:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		register int save_adj = 0;
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (blessed) save_adj = -5;
		if (cursed)
		    save_adj = 5;

		if (save_throw(VS_MAGIC - save_adj, tp)) {
		    msg("Nothing happens.");
		}
		else
		     turn_on (*tp, ISHUH);
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	when WS_PARALYZE:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		register int save_adj = 0;
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (blessed) save_adj = -5;
		if (cursed)
		    save_adj = 5;

		if (save_throw(VS_MAGIC - save_adj, tp)) {
		    msg("Nothing happens.");
		}
		else {
		    tp->t_no_move = FREEZETIME;
		}
	    }
	when WS_MDEG:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (cursed) {
		     tp->t_stats.s_hpt *= 2;
		     msg("The %s appears to be stronger now!", 
			monsters[tp->t_index].m_name);
		}
		else {
		     if (on(*tp, CANSELL)) {
			luck++;
			aggravate();
		     }
		     tp->t_stats.s_hpt /= (blessed ? 4 : 2);
		     msg("The %s appears to be %sweaker now.", 
			monsters[tp->t_index].m_name, (blessed ? "much " : "") );
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	        if (tp->t_stats.s_hpt < 1)
		     killed(&player, item, TRUE, TRUE);
	    }
	when WS_ANNIH:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (cursed) {
		    register int m1, m2;
		    coord mp;
		    struct linked_list *titem;
		    char ch;
		    struct thing *th;
		    for (m1=tp->t_pos.x-1 ; m1 <= tp->t_pos.x+1 ; m1++) {
			for(m2=tp->t_pos.y-1 ; m2<=tp->t_pos.y+1 ; m2++) {
			    ch = winat(m2,m1);
			    if (shoot_ok(ch) && ch != PLAYER) {
				mp.x = m1;	/* create it */
				mp.y = m2;
				titem = new_item(sizeof(struct thing));
				new_monster(titem,(short)tp->t_index,&mp,FALSE);
				th = THINGPTR(titem);
				turn_on (*th, ISMEAN);
				runto(&mp,&hero);
			    }
			}
		    }
		    delta.y = y;
		    delta.x = x;
		    turn_on (*tp, ISMEAN);
		    runto(&delta, &hero);
		}
		else { /* if its a UNIQUE it might still live */
		    tp = THINGPTR(item);
		    if (on(*tp, ISUNIQUE) && save_throw(VS_MAGIC, tp)) {
			tp->t_stats.s_hpt /= 2;
			if (tp->t_stats.s_hpt < 1) {
			     killed(&player, item, FALSE, TRUE);
			     msg("You have disintegrated the %s.", 
				    monsters[tp->t_index].m_name);
			}
			else {
			    delta.y = y;
			    delta.x = x;
			    runto(&delta, &hero);
			    msg("The %s appears wounded.",
				monsters[tp->t_index].m_name);
			    summon(tp);
			}
		    }
		    else {
			if (on(*tp, CANSELL)) {
			    luck++;
			    aggravate();
			}
			msg("You have disintegrated the %s.", 
				monsters[tp->t_index].m_name);
			killed (&player, item, FALSE, TRUE);
		    }
		}
	    }
	when WS_NOTHING:
	    msg("Nothing %shappens.", (blessed ? "good " : (cursed ? "bad " : "")) );
	when WS_INVIS: {
	    if (cursed) {
		register int x1, y1, x2, y2;
		bool zapped = FALSE;
		if ((rp = roomin(&hero)) == NULL) {
		    x1 = max(hero.x - 1, 0);
		    y1 = max(hero.y - 1, 0);
		    x2 = min(hero.x + 1, COLS - 1);
		    y2 = min(hero.y + 1, LINES - 3);
		}
		else {
		    x1 = rp->r_pos.x;
		    y1 = rp->r_pos.y;
		    x2 = rp->r_pos.x + rp->r_max.x;
		    y2 = rp->r_pos.y + rp->r_max.y;
		}
		for (item = mlist; item != NULL; item = next(item)) {
		    tp = THINGPTR(item);
		    if (tp->t_pos.x >= x1 && tp->t_pos.x <= x2 &&
		    	tp->t_pos.y >= y1 && tp->t_pos.y <= y2) {
			turn_on(*tp, ISINVIS);
			turn_on(*tp, ISRUN);
			turn_off(*tp, ISDISGUISE);
			runto(&tp->t_pos, &hero);
			zapped = TRUE;
		    }
		}
		if (zapped)
		    msg("The monsters seem to have all disappeared.");
		else
		    msg("Nothing happens.");
	    }
	    else {
		y = hero.y;
		x = hero.x;
		while (shoot_ok(winat(y, x)))
		{
		    y += delta.y;
		    x += delta.x;
		}
		if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		    item = find_mons(y, x);
		    tp = THINGPTR(item);
		    if (blessed) {
			turn_off(*tp, ISINVIS);
			turn_off(*tp, ISSHADOW);
			msg("The %s appears.", monsters[tp->t_index].m_name);
		    }
		    else {
			turn_on(*tp, ISINVIS);
			msg("The %s disappears.", 
				monsters[tp->t_index].m_name);
		    }
		}
		else
		    msg("Nothing happens.");
	    }
	    light(&hero);
	}
	when WS_BLAST: {
	    register char ch;
	    register struct linked_list *item, *ip;
	    register struct object *obj, *op;    
	    register struct trap *trp;

	    item = spec_item(WEAPON, GRENADE, 0, 0);
	    obj = (struct object *) ldata(item);
	    obj->o_count = 1;
	    obj->o_flags |= ISKNOW;
	    msg("BOOOM!");
	    aggravate();
	    obj->o_pos.x = hero.x;
	    obj->o_pos.y = hero.y;
	    for ( ; ; ) {
		obj->o_pos.y += delta.y;
		obj->o_pos.x += delta.x;
		if (!ce(obj->o_pos, hero) &&
			cansee(unc(obj->o_pos)) &&
			mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
		    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, 
		   		show(obj->o_pos.y, obj->o_pos.x));
		}
		if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) 
			&& ch != DOOR && !ce(obj->o_pos, hero)) {
		    if (cansee(unc(obj->o_pos)) && 
			    ntraps + 1 < MAXTRAPS + MAXTRAPS &&
			    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
			mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, TRAPDOOR);
			draw(cw);
		    }
		    if (isatrap(ch)) {
			trp = trap_at(obj->o_pos.y, obj->o_pos.x);
			if (trp != NULL) {
			    trp->tr_type = TRAPDOOR;
			    trp->tr_flags |= ISFOUND;
			    trp->tr_show = TRAPDOOR;
			}
		    }
		    else if (isalpha(ch)) 
			hit_monster(unc(obj->o_pos), obj, &player);
		    else if ((ch == FLOOR || ch == PASSAGE)
			     && ntraps + 1 < MAXTRAPS + MAXTRAPS) {
			mvaddch(obj->o_pos.y, obj->o_pos.x, TRAPDOOR);
			traps[ntraps].tr_type = TRAPDOOR;
			traps[ntraps].tr_flags = ISFOUND;
			traps[ntraps].tr_show = TRAPDOOR;
			traps[ntraps].tr_pos.y = obj->o_pos.y;
			traps[ntraps++].tr_pos.x = obj->o_pos.x;
		    }
		    else if (ch == POTION || ch == SCROLL || ch == FOOD
			    || ch == WEAPON || ch == RING || ch == ARMOR
			    || ch == STICK || ch == ARTIFACT || ch == GOLD) {
			if ((ip = find_obj(obj->o_pos.y, obj->o_pos.x))
					!= NULL) {
			    detach(lvl_obj, ip);
			    discard(ip);
			}
			mvaddch(obj->o_pos.y, obj->o_pos.x,
				roomin(&obj->o_pos) == NULL ? PASSAGE : FLOOR);
		    }
		    continue;
		}
		break;
	    }
	    discard(item);
	}
	otherwise:
	    msg("What a bizarre schtick!");
    }
}

/*
 * drain:
 *	Do drain hit points from player shtick
 */

drain(ymin, ymax, xmin, xmax)
int ymin, ymax, xmin, xmax;
{
    register int i, j, count;
    register struct thing *ick;
    register struct linked_list *item;

    /*
     * First count how many things we need to spread the hit points among
     */
    count = 0;
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)))
		count++;
    if (count == 0)
    {
	msg("You have a tingling feeling.");
	return;
    }
    count = pstats.s_hpt / count;
    pstats.s_hpt /= 2;
    /*
     * Now zot all of the monsters
     */
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)) &&
	        ((item = find_mons(i, j)) != NULL))
	    {
		ick = THINGPTR(item);
		if (on(*ick, CANSELL)) {
		    luck++;
		    aggravate();
		}
		if ((ick->t_stats.s_hpt -= count) < 1)
		    killed(&player, item, 
			   cansee(i, j) && !on(*ick, ISINVIS), TRUE);
	    }
}

/*
 * charge a wand for wizards.
 */
char *
charge_str(obj)
register struct object *obj;
{
    static char buf[20];

    if (!(obj->o_flags & ISKNOW))
	buf[0] = '\0';
    else if (terse)
	sprintf(buf, " [%d]", obj->o_charges);
    else if (obj->o_charges == 1)
	sprintf(buf, " [%d charge]", obj->o_charges);
    else
	sprintf(buf, " [%d charges]", obj->o_charges);
    return buf;
}


/*
 * shoot_bolt fires a bolt from the given starting point in the
 * 	      given direction
 */

bool
shoot_bolt(shooter, start, dir, get_points, reason, name, damage)
struct thing *shooter;
coord start, dir;
bool get_points;		/* should player get exp points? */
short reason;			/* reason for dying */
char *name;			/* fire, nerve, cold, etc */
int damage;			/* make zapee suffer */
{
    register char dirch, ch;
    register bool change;
    register short y, x;
    coord pos;
    coord spotpos[BOLT_LENGTH];
    bool ret_val = FALSE;	/* True if breathing monster gets killed */
    register struct linked_list *item;
    register struct thing *tp;
    register char *mname;	/* = monsters[tp->t_index].m_name */
    int bounced;		/* where along BOLT_LENGTH it hit a wall */
    int player_damage;		/* damage if player saved */
    bool no_effect;		/* zap does not effect zapee */
    int take_that[BOLT_LENGTH+1]; /* damage to each monster */
				/* last spot for player */

    debug("%s does %d damage", name, damage);

    switch (dir.y + dir.x) {
	when 0: dirch = '/';
	when 1: case -1: dirch = (dir.y == 0 ? '-' : '|');
	when 2: case -2: dirch = '\\';
    }
    pos.y = start.y + dir.y;
    pos.x = start.x + dir.x;
    change = FALSE;
    for (y = 0; y < BOLT_LENGTH+1; y++) take_that[y] = 0;

    bounced = 0;
    for (y = 0; y < BOLT_LENGTH; y++)
    {
	no_effect = FALSE;
	ch = winat(pos.y, pos.x);
	spotpos[y] = pos;

	/* Bolt damage dimishes over space */
	damage = max(0,damage-(y/3));

	/* Are we at hero? */
	if (ce(pos, hero)) goto at_hero;

	switch (ch)
	{
	    case SECRETDOOR:	/* Bolts pass through regular doors */
	    case '|':
	    case '-':
	    case ' ':
		bounced = y;
		if (dirch == '-' || dirch == '|') {
		    dir.y = -dir.y;
		    dir.x = -dir.x;
		}
		else switch (ch) {
		    case '|':
		    case '-':
		    case SECRETDOOR:
			{
			    register struct room *rp;

			    rp = roomin(&pos);
			    if (pos.y == rp->r_pos.y ||
				pos.y == rp->r_pos.y + rp->r_max.y - 1) {
				dir.y = -dir.y;
				change ^= TRUE;
			    }
			    if (pos.x == rp->r_pos.x ||
				pos.x == rp->r_pos.x + rp->r_max.x - 1) {
				dir.x = -dir.x;
				change ^= TRUE;
			    }
			}
			break;
		    default:	/* A wall */
			{
			    char chy = mvinch(pos.y-dir.y, pos.x+dir.x),
				 chx = mvinch(pos.y+dir.y, pos.x-dir.x);

			    if (chy != WALL && chy != SECRETDOOR &&
				chy != '-' && chy != '|') {
				dir.y = -dir.y;
				change = TRUE;
			    }
			    else if (chx != WALL && chx != SECRETDOOR &&
				     chx != '-' && chx != '|') {
				    dir.x = -dir.x;
				    change = TRUE;
				}
			    else {
				dir.y = -dir.y;
				dir.x = -dir.x;
			    }
			}
		}

		/* Do we change how the bolt looks? */
		if (change) {
		    change = FALSE;
		    if (dirch == '\\') dirch = '/';
		    else if (dirch == '/') dirch = '\\';
		}

		y--;
		msg("The %s bounces!", name);
		break;
	    default:
		if (isalpha(ch)) {			/* hit a monster */

		    item = find_mons(unc(pos));
		    if (item == NULL) {
			debug("Can't find monster %c @ %d %d.", ch, unc(pos));
			continue;
		    }
		    tp = THINGPTR(item);
		    mname = monsters[tp->t_index].m_name;

		    /* Disguised monsters stay hidden if they save */
		    if (on(*tp, ISDISGUISE) && save_throw(VS_MAGIC, tp) &&
			(tp->t_type != tp->t_disguise) && off(player, ISBLIND)) {
			msg("Wait! That's a %s!", mname);
			turn_off(*tp, ISDISGUISE);
		    }

		    tp->t_wasshot = TRUE;
		    if (on(*tp, CANSELL)) {
			luck++;
			aggravate();
		    }

		    /* Hit the monster -- does it do damage? */
		    if (strcmp(name, "ice") == 0) {
			if ( on(*tp, NOCOLD) || on(*tp, ISUNDEAD) )
			    no_effect=TRUE;
		    }
		    else if (strcmp(name, "flame") == 0) {
			if ( on(*tp, NOFIRE) )
			    no_effect=TRUE;
			if ( on(*tp, CANBBURN) ) {
			    msg("The %s is burned to death by the flame.",
				mname);
			    take_that[y] += tp->t_stats.s_hpt + 1;
			    ret_val = TRUE;
			}
		    }
		    else if (strcmp(name, "lightning bolt") == 0) {
			if ( on(*tp, NOBOLT) )
			    no_effect=TRUE;
			if ( on(*tp, BOLTDIVIDE) ) {
			    no_effect=TRUE;
			    if (creat_mons(tp, tp->t_index, FALSE))
				msg("The %s divides the %s.", name, mname);
			}
		    }
		    if (no_effect == TRUE) {
			msg("The %s has no effect on the %s.", name,
			    on(player, ISBLIND) ? "monster" : mname);
		    }
		    else {
			take_that[(bounced?bounced--:y)] +=
				save_throw(VS_MAGIC, tp) ? damage/2 : damage;
		    }
		}
		else if (pos.y == hero.y && pos.x == hero.x) {
at_hero: 
		    player_damage = damage;
		    running = fighting = FALSE;
		    bounced = 0;

		    if (cur_armor != NULL && 
			    cur_armor->o_which == CRYSTAL_ARMOR &&
			    (strcmp(name, "acid") == 0)) {
			    player_damage = 0;
			    msg("The acid splashes harmlessly against your armor!");
		    }
		    else if (((cur_armor != NULL &&
				cur_armor->o_which == CRYSTAL_ARMOR) ||
			    ISWEARING(R_ELECTRESIST)) &&
			    (strcmp(name, "lightning bolt") == 0)) {
			    player_damage = 0;
			    if ( rnd(100) < 75 
				    && cur_weapon != NULL
				    && shooter != &player) {
				cur_weapon->o_flags |= ISZAPPED;
				cur_weapon->o_charges += (10 + rnd(15));
			    }
			    if (cur_weapon != NULL && cur_armor != NULL)
				msg("Your armor and %s are covered with dancing blue lights!", weaps[cur_weapon->o_which].w_name);
			    else if (cur_armor != NULL)
				msg("Your armor is covered with dancing blue lights!");
			    else if (cur_weapon != NULL)
				msg("Your %s is covered with dancing blue lights!", weaps[cur_weapon->o_which].w_name);
			    else
				msg("You are momentarily covered with dancing blue lights.");
		    }
		    else if(ISWEARING(R_FIRERESIST) &&
			    (strcmp(name, "flame") == 0)) {
			    player_damage = 0;
			    msg("The flame bathes you harmlessly.");
		    }
		    else if(ISWEARING(R_COLDRESIST) &&
			    (strcmp(name, "ice") == 0)) {
			    player_damage = 0;
			    msg("The ice cracks and quickly melts around you.");
		    }
		    else if (save(VS_MAGIC)) {
			take_that[BOLT_LENGTH] += player_damage/2;
			debug("player dodges %s for %d.", name,player_damage/2);
		    }
		    else {
			debug("player zapped by %s for %d.", name, player_damage);
			take_that[BOLT_LENGTH] += player_damage;
		    }

		    /* Check for gas with special effects */
		    if (!save(VS_BREATH) && !ISWEARING(R_BREATHE)) {
			if (strcmp(name, "nerve gas") == 0) {
			    if (no_command == 0) {
				msg("The nerve gas paralyzes you.");
				no_command = FREEZETIME;
			    }
			}
			else if (strcmp(name, "sleeping gas") == 0) {
			    if (no_command == 0) {
				msg("The sleeping gas puts you to sleep.");
				no_command = SLEEPTIME;
			    }
			}
			else if (strcmp(name, "slow gas") == 0
				&& !ISWEARING(R_FREEDOM)) {
			    msg("You feel yourself moving %sslower.",
				    on(player, ISSLOW) ? "even " : "");
			    if (on(player, ISSLOW))
				lengthen(noslow, rnd(10) + 4);
			    else {
				turn_on(player, ISSLOW);
				player.t_turn = TRUE;
				fuse(noslow, 0, rnd(10)+4, AFTER);
			    }
			}
			else if (strcmp(name, "fear gas") == 0) {
			    if (!(on(player, ISFLEE) &&
				  (player.t_dest == &shooter->t_pos)) &&
				(shooter != &player)) {
				if (off(player, SUPERHERO)) {
				    turn_on(player, ISFLEE);
				    player.t_dest = &shooter->t_pos;
				    msg("The fear gas terrifies you.");
				}
				else
				    msg("The fear gas has no effect.");
			    }
			}
		    }
		}
		mvwaddch(cw, pos.y, pos.x, dirch);
		draw(cw);
	}
	pos.y += dir.y;
	pos.x += dir.x;
    }

    /*
     * Now that we have determined the damage for the length of the bolt,
     * apply it to each monster (and then the player) in turn
     */
    for (x = 0; x < BOLT_LENGTH; x++) {
	ch = winat(spotpos[x].y, spotpos[x].x);

	if (take_that[x] != 0 && isalpha(ch)) {
	    if ((item = find_mons(unc(spotpos[x]))) == NULL) {
		debug("Can't find monster %c @ %d %d.", ch, unc(pos));
		continue;
	    }
	    else tp = THINGPTR(item);
	    mname = monsters[tp->t_index].m_name;

	    debug("Zapped %s for %d (%d)",
		mname, take_that[x], tp->t_stats.s_hpt-take_that[x]);
	    if ((tp->t_stats.s_hpt -= take_that[x]) <= 0) {
		msg("The %s kills the %s.", name,
		    on(player, ISBLIND) ? "monster" : mname);
		killed(NULL, item, FALSE, get_points);
		ret_val = TRUE;
	    }
	    else if (get_points)
	    {
		runto(&spotpos[x], &hero);
		summon(tp);
	    }
	}
	if ( spotpos[x].y >= 0 && spotpos[x].x >= 0 &&
			spotpos[x].y < LINES && spotpos[x].x < COLS)
	    mvwaddch(cw, spotpos[x].y, spotpos[x].x,
			show(spotpos[x].y, spotpos[x].x));
    }
    if (take_that[BOLT_LENGTH] != 0) {
	if (terse)	msg("The %s hits you.", name);
	else		msg("You are hit by the %s.", name);
	if ((pstats.s_hpt -= take_that[BOLT_LENGTH]) <= 0) {
	    death(reason);
	    return;
	}
    }

    return(ret_val);
}

/*
 * Last Modified: 08/12/86
 */
/*
 * Functions for dealing with problems brought about by weapons
 *
 */

#include <ctype.h>
#include "rogue.h"

/*
 * missile:
 *	Fire a missile in a given direction
 */

missile(ydelta, xdelta, item, tp)
int ydelta, xdelta;
register struct linked_list *item;
register struct thing *tp;
{
    register struct object *obj;
    register struct linked_list *nitem;

    /*
    * Get which thing we are hurling
    */
    if (item == NULL) {
	return;
    }
    obj = OBJPTR(item);
    if (!dropcheck(obj) || is_current(obj)) {
	return;
    }
    /*
    * Get rid of the thing. If it is a non-multiple item object, or
    * if it is the last thing, just drop it. Otherwise, create a new
    * item with a count of one.
    */
    if (obj->o_count < 2)
      {
	if (tp->t_pack == pack)
	  rem_pack(obj);
	else
	  detach(tp->t_pack, item);
      }
    else {
	obj->o_count--;
	nitem = (struct linked_list *) new_item(sizeof *obj);
	obj = OBJPTR(nitem);
	*obj = *(OBJPTR(item));
	obj->o_count = 1;
	item = nitem;
    }
    if (obj->o_type == ARTIFACT)
	has_artifact &= ~(1 << obj->o_which);
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE) {
	if (obj->o_flags & ISBLESSED)
	    obj->o_flags &= ~ISBLESSED;
	else 
	    obj->o_flags |= ISCURSED;
    }
    updpack (FALSE);
    do_motion(obj, ydelta, xdelta, tp);

   /*
    * AHA! Here it has hit something. If it is a wall or a door,
    * or if it misses (combat) the monster, put it on the floor
    */
    if (!hit_monster(obj->o_pos.y, obj->o_pos.x, obj, tp)) {
	if (obj->o_type == WEAPON && obj->o_which == GRENADE) {
	    static coord fpos;

	    msg("BOOOM!");
	    aggravate();
	    if (ntraps+1 < 2*MAXTRAPS && fallpos(&obj->o_pos, &fpos, TRUE)) {
		mvaddch(fpos.y, fpos.x, TRAPDOOR);
		traps[ntraps].tr_type = TRAPDOOR;
		traps[ntraps].tr_flags = ISFOUND;
		traps[ntraps].tr_show = TRAPDOOR;
		traps[ntraps].tr_pos.y = fpos.y;
		traps[ntraps++].tr_pos.x = fpos.x;
		light(&hero);
	    }
	    discard(item);
	}
	else if (obj->o_flags & ISLOST) {
	    if (obj->o_type == WEAPON)
		addmsg("The %s", weaps[obj->o_which].w_name);
	    else
		addmsg(inv_name(obj, TRUE));
	    msg(" vanishes in a puff of greasy smoke.");
	    discard(item);
	}
	else {
	    fall(&player, item, TRUE);
	    if (obj->o_flags & CANRETURN)
		msg("You have %s.", inv_name(obj, TRUE));
	}
    }
    else if (obj->o_flags & ISOWNED) {
	add_pack(item, TRUE);
	msg("You have %s.", inv_name(obj, TRUE));
    }
    mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
do_motion(obj, ydelta, xdelta, tp)
register struct object *obj;
register int ydelta, xdelta;
register struct thing *tp;
{

   /*
    * Come fly with us ...
    */
    obj->o_pos = tp->t_pos;
    for ( ; ;) {
	register int ch;
	/*
	 * Erase the old one
	 */
	if (!ce(obj->o_pos, tp->t_pos) &&
	    cansee(obj->o_pos.y, obj->o_pos.x) &&
	    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
		mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, show(obj->o_pos.y, obj->o_pos.x));
	}
	/*
	 * Get the new position
	 */
	obj->o_pos.y += ydelta;
	obj->o_pos.x += xdelta;
	if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR && !ce(obj->o_pos, hero)) {
	    /*
	     * It hasn't hit anything yet, so display it
	     * If it alright.
	     */
	    if (cansee(obj->o_pos.y, obj->o_pos.x) &&
		mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
		    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type);
		    wrefresh(cw);
	    }
	    continue;
	}
	break;
    }
}

/*
 * fall:
 *	Drop an item someplace around here.
 */

fall(tp, item, pr)
register struct linked_list *item;
register struct thing *tp;
bool pr;
{
    register struct object *obj;
    register struct room *rp;
    static coord fpos;

    obj = OBJPTR(item);
    rp = roomin(&(tp->t_pos));
    if (obj->o_flags & CANRETURN) {
	add_pack(item, TRUE);
	return;
    }
    else if (fallpos(&obj->o_pos, &fpos, TRUE)) {
 	if (obj->o_flags & CANBURN && obj->o_type == WEAPON
 		&& obj->o_which == MOLOTOV
		&& ntraps + 1 < 2*MAXTRAPS) {
	    mvaddch(fpos.y, fpos.x, FIRETRAP);
	    traps[ntraps].tr_type = FIRETRAP;
	    traps[ntraps].tr_flags = ISFOUND;
	    traps[ntraps].tr_show = FIRETRAP;
	    traps[ntraps].tr_pos.y = fpos.y;
	    traps[ntraps++].tr_pos.x = fpos.x;
	    if (rp != NULL)
		rp->r_flags &= ~ISDARK;
	}
	else {
	    mvaddch(fpos.y, fpos.x, obj->o_type);
	    obj->o_pos = fpos;
	    attach(lvl_obj, item);
	}
	if (rp != NULL &&
	    (!(rp->r_flags & ISDARK) ||
	    (rp->r_flags & HASFIRE))) {
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	}
	return;
    }
    if (pr) {
	if (cansee(obj->o_pos.y, obj->o_pos.x)) {
	    if (obj->o_type == WEAPON)
		addmsg("The %s", weaps[obj->o_which].w_name);
	    else
		addmsg(inv_name(obj, TRUE));
	    msg(" vanishes as it hits the ground.");
	}
    }
    discard(item);
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */

init_weapon(weap, type)
register struct object *weap;
char type;
{
    register struct init_weps *iwp = &weaps[type];

    weap->o_damage = iwp->w_dam;
    weap->o_hurldmg = iwp->w_hrl;
    weap->o_launch = iwp->w_launch;
    weap->o_flags = iwp->w_flags;
    weap->o_weight = iwp->w_wght;
    if (weap->o_flags & ISMANY) {
	    weap->o_count = rnd(8) + 8;
	    weap->o_group = ++group;
    }
    else weap->o_count = 1;
}

/*
 * Does the missile hit the monster
 */

hit_monster(y, x, obj, tp)
register int y, x;
struct object *obj;
register struct thing *tp;
{
    static coord mp;

    mp.y = y;
    mp.x = x;
    if (tp == &player) {
	/* Make sure there is a monster where it landed */
	if (!isalpha(mvwinch(mw, y, x))) {
		return(FALSE);
	}
	return(fight(&mp, winat(y, x), obj, TRUE));
    } 
    else {
	if ((good_monster(*tp)) && ce(mp, hero)) {
	    if (on(*tp, ISFAMILIAR))
		msg("Please get out of the way, Master!  I nearly hit you.");
	    else
		msg("Get out of the way %s! I nearly hit you.", whoami);
	    return(FALSE);
	}
	if (!isalpha(mvwinch(mw, y, x))) 
	    return FALSE;
	if (ce(mp, hero))
		return attack(tp, obj, TRUE);
	else {
	    register struct linked_list *mon;
	    if ((mon = find_mons(y, x)) != NULL)
		return mon_mon_attack(tp, mon, obj, TRUE);
	}
    }
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char *num(n1, n2)
register int n1, n2;
{
    static char numbuf[2*LINELEN];

    if (n1 == 0 && n2 == 0)
	return "+0";

    if (n2 == 0)
	 sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    else sprintf(numbuf, "%s%d, %s%d", n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2);

    return(numbuf);
}

/*
 * wield:
 *	Pull out a certain weapon
 */

wield()
{
    register struct linked_list *item;
    register struct object *obj, *oweapon;

    oweapon = cur_weapon;
    if (!dropcheck(cur_weapon)) {
	cur_weapon = oweapon;
	return;
    }
    cur_weapon = oweapon;
    if ((item = get_item("wield", WEAPON)) == NULL) {
	after = FALSE;
	return;
    }
    obj = OBJPTR(item);
    if (!wield_ok(&player, obj, TRUE))
	return;

    if (is_current(obj)) {
	after = FALSE;
	return;
    }
    
    msg("%sielding %s.", (terse ? "W" : "You are now w"), inv_name(obj, TRUE));
    cur_weapon = obj;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
fallpos(pos, newpos, passages)
register coord *pos, *newpos;
register bool passages;
{
    register int y, x, cnt, ch;

    cnt = 0;
    for (y = pos->y - 1 ; y <= pos->y + 1 ; y++)
	for (x = pos->x - 1 ; x <= pos->x + 1 ; x++)
	    /*
	     * check to make certain the spot is empty, if it is,
	     * put the object there, set it in the level list
	     * and re-draw the room if he can see it
	     */
	    if (y == hero.y && x == hero.x)
		continue;
	    else if (((ch = winat(y, x)) == FLOOR ||
		(passages && ch == PASSAGE)) && rnd(++cnt) == 0) {
		    newpos->y = y;
		    newpos->x = x;
	    }
    return (cnt != 0);
}

/*
 * wield_ok:
 *	enforce player class weapons restrictions
 */

bool
wield_ok(wieldee, obj, print_message)
struct thing *wieldee;
register struct object *obj;
bool print_message;
{
    register int which = obj->o_which;
    register bool ret_val = TRUE;
    int class_type = wieldee->t_ctype;

    if (obj->o_type != WEAPON) {
	ret_val = FALSE;
	return(ret_val);
    }

    else switch(class_type) {
    when C_MAGICIAN:			/* need one hand free */
    case C_ILLUSION:
	if (obj->o_flags & ISTWOH)
	    ret_val = FALSE;
    when C_THIEF:			/* need portable weapon  */
    case C_ASSASIN:
    case C_NINJA:
	if (obj->o_flags & ISTWOH)
	    ret_val = FALSE;
    when C_CLERIC:			/* No sharp weapons */
	if (obj->o_flags & ISSHARP)
	    ret_val = FALSE;
    when C_DRUID:			/* No non-silver metal weapons */
	if (obj->o_flags & ISMETAL && ~(obj->o_flags & ISSILVER))
	    ret_val = FALSE;
    when C_PALADIN:			/* must wield sharp stuff */
	if ((obj->o_flags & ISSHARP) == FALSE)
	    ret_val = FALSE;
    when C_FIGHTER:			/* wield anything */
    case C_RANGER:
    case C_MONSTER:

    otherwise:				/* Unknown class */
	debug("Unknown class %d.", class_type);
	break;
    }

    if (itemweight(obj) > 18 * pstats.s_str) {
	if (wieldee == &player && print_message==TRUE)
	    msg("That is too heavy for you to swing effectively!");
	ret_val = FALSE;
	return(ret_val);
    }

    if (ret_val==FALSE && print_message==MESSAGE)
	switch(class_type) {
	when C_MAGICIAN:
	case C_ILLUSION:
	    msg("You'll find it hard to cast spells while wielding that!");
	when C_THIEF:
	case C_ASSASIN:
	case C_NINJA:
	    msg("Don't expect to backstab anyone while wielding that!");
	when C_CLERIC:
	case C_DRUID:
	case C_PALADIN:
	    msg("Your god strongly disapproves of your wielding that!");
	case C_FIGHTER:
	case C_RANGER:
	case C_MONSTER:
		break;
	}
    return(ret_val);
}

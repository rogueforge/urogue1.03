/*
 * Last Modified: 08/10/86
 */
/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 */

#include <ctype.h>
#include "rogue.h"
#include "mach_dep.h"
#if defined(attron)
#include <term.h>
#endif attron

/*
 * whatis:
 *	What a certain object is
 */

whatis(what)
struct linked_list *what;
{
    register struct object *obj;
    register struct linked_list *item;

    if (what == NULL) {		/* do we need to ask which one? */
	if ((item = get_item("identify", 0)) == NULL)
	    return;
    }
    else
	item = what;
    obj = OBJPTR(item);
    obj->o_flags |= ISKNOW;
    switch (obj->o_type) {
        when SCROLL:
	    s_know[obj->o_which] = TRUE;
	    if (s_guess[obj->o_which]) {
		free(s_guess[obj->o_which]);
		s_guess[obj->o_which] = NULL;
	    }
        when POTION:
	    p_know[obj->o_which] = TRUE;
	    if (p_guess[obj->o_which]) {
		free(p_guess[obj->o_which]);
		p_guess[obj->o_which] = NULL;
	    }
	when STICK:
	    ws_know[obj->o_which] = TRUE;
	    if (ws_guess[obj->o_which]) {
		free(ws_guess[obj->o_which]);
		ws_guess[obj->o_which] = NULL;
	    }
        when WEAPON:
        case ARMOR:
        when RING:
	    r_know[obj->o_which] = TRUE;
	    if (r_guess[obj->o_which]) {
		free(r_guess[obj->o_which]);
		r_guess[obj->o_which] = NULL;
	    }
    }
    if (what == NULL)
	msg(inv_name(obj, FALSE));
}

/*
 * create_obj:
 *	Create any object.
 */
create_obj(which_item, which_type, flags)
int flags;
int which_item, which_type;
{
    reg struct linked_list *item;
    reg struct object *obj;
    reg int wh;
    reg char ch, newitem, newtype, whc, msz, *pt;
    WINDOW *thiswin;
    int tsize;
    int pres_item = 1;
    bool cursed  = flags & ISCURSED;

    thiswin = cw;
    if (which_item == 0) {
	bool nogood = TRUE;
	char buf[2*LINELEN];
	thiswin = hw;
	msg("What do you want to create (* for list)? ");
	do {
	    ch = readchar();
	    if (ch == '*') {
		sprintf(buf,"ITEM\tKEY");
		add_line(buf);
		sprintf(buf,"%s\t %c", things[TYP_RING].mi_name,RING);
		add_line(buf);
		sprintf(buf,"%s\t %c", things[TYP_STICK].mi_name,STICK);
		add_line(buf);
		sprintf(buf,"%s\t %c", things[TYP_POTION].mi_name,POTION);
		add_line(buf);
		sprintf(buf,"%s\t %c", things[TYP_SCROLL].mi_name,SCROLL);
		add_line(buf);
		sprintf(buf,"%s\t %c", things[TYP_ARMOR].mi_name,ARMOR);
		add_line(buf);
		sprintf(buf,"%s\t %c", things[TYP_WEAPON].mi_name,WEAPON);
		add_line(buf);
		sprintf(buf,"%s\t %c",things[TYP_FOOD].mi_name,FOOD);
		add_line(buf);
		if (wizard) {
		    sprintf(buf,"%s %c", things[TYP_ARTIFACT].mi_name, ARTIFACT);
		    add_line(buf);
		    sprintf(buf,"monster\t m");
		    add_line(buf);
		}
		end_line();
		msg("");
		msg("What do you want to create (* for list) ? ");
	    }
	    if(ch == ESCAPE) 
		return;
	    switch (ch) {
	    when RING:
	    case STICK:	
	    case POTION:
	    case SCROLL:	
	    case ARMOR:	
	    case WEAPON:
	    case FOOD:
		nogood = FALSE;
	    when 'm':
	    case ARTIFACT:
		if (wizard)
		    nogood = FALSE;
	    otherwise:
		nogood = TRUE;
	    }
	} while (nogood == TRUE);
	newitem = ch;
    }
    else newitem = which_item;

    pt = "those";
    msz = 0;
    switch(newitem) {
    	when 'm':
	    makemon();		/* make monster and be done with it */
	    return;
	when GOLD:
	    pt = "gold";
	when POTION:	whc = TYP_POTION;	tsize = msz = maxpotions;
	when SCROLL:	whc = TYP_SCROLL;	tsize = msz = maxscrolls;
	when FOOD:	whc = TYP_FOOD;		tsize = msz = maxfoods;
	when WEAPON:	whc = TYP_WEAPON;	tsize = msz = maxweapons;
	when ARMOR:	whc = TYP_ARMOR;	tsize = msz = maxarmors;
	when RING:	whc = TYP_RING;		tsize = msz = maxrings;
	when STICK:	whc = TYP_STICK;	tsize = msz = maxsticks;
	when ARTIFACT:	whc = TYP_ARTIFACT;	tsize = msz = maxartifact;
	otherwise:
	    if (thiswin == hw)
		restscr(cw);
	    mpos = 0;
	    msg("Even wizards can't create %s !!",pt);
	    return;
    }
    if(msz == 1)		/* if only one type of item */
	ch = 1;
    else if (which_type == 0) {
	struct magic_item *wmi;
	struct init_artifact *war;
	char wmn;
	char buf[2*LINELEN];
	int ii;
	char numstr[20];

	mpos = 0;
	wmi = NULL;
	war = NULL;
	wmn = 0;
	switch(newitem) {
	    when POTION:	wmi = p_magic;
	    when SCROLL:	wmi = s_magic;
	    when RING:		wmi = r_magic;
	    when STICK:		wmi = ws_magic;
	    when FOOD:		wmi = fd_data;
	    when ARTIFACT:	war = arts;
	    when WEAPON:	wmn = 1;
	    when ARMOR:		wmn = 2;
	}
	thiswin = hw;
	for (ii = 0 ; ii < msz && pres_item <= tsize; ii++) {
	    sprintf(buf,"%d) ", pres_item);
	    if (wmi != NULL) {
		strcat(buf, wmi->mi_name);
		wmi++;
	    }
	    else if (war != NULL) {
		strcat(buf, war->ar_name);
		war++;
	    }
	    else if(wmn == 1)
		strcat(buf, weaps[pres_item - 1].w_name);
	    else if(wmn == 2)
		strcat(buf, armors[pres_item - 1].a_name);
	    add_line(buf);
	    pres_item++;
	}
	end_line();
	msg("");
	msg("Which %s? ",things[whc].mi_name);
	do {
	    ch = get_str(numstr, cw);
	    if (ch == QUIT || ch == ESCAPE) {
	        msg("");
	        return;
	    }
	    newtype = atoi(numstr) - 1;
	    if (newtype < 0 || newtype >= tsize) {
		msg("Please enter a number in the displayed range: ");
	    }
	} until (newtype >= 0 && newtype < tsize);
    }
    else newtype = which_type;
    item = new_item(sizeof *obj);	/* get some memory */
    obj = OBJPTR(item);
    if (newitem == ARTIFACT) {
	new_artifact(newtype, obj);
	if (add_pack(item, FALSE) == FALSE) {
	    obj->o_pos = hero;
	    fall(&player, item, TRUE);
	}
	return;
    }
    obj->o_type = newitem;		/* store the new items */
    obj->o_mark[0] = '\0';
    obj->o_which = newtype;
    obj->o_group = 0;
    obj->o_count = 1;
    obj->o_flags = 0;
    obj->o_dplus = obj->o_hplus = 0;
    obj->o_weight = 0;
    wh = obj->o_which;
    mpos = 0;
    if (wizard)					/* wizard gets to choose */
	 whc = getbless();
    else whc = (cursed ? -1 : 1) * rnd(4);
    if (whc < 0)
	obj->o_flags |= ISCURSED;
    else if (whc > 0 && obj->o_type != WEAPON && obj->o_type != ARMOR)
	obj->o_flags |= ISBLESSED;
    obj->o_flags |= ISKNOW;

    switch (obj->o_type) {
    when WEAPON:
	init_weapon(obj, wh);
	obj->o_hplus += whc;
	obj->o_dplus += whc;
    when ARMOR:
	obj->o_weight = armors[wh].a_wght;
	obj->o_ac = armors[wh].a_class - whc;
    when RING:
	r_know[wh] = TRUE;
	obj->o_ac = whc;
	obj->o_weight = things[TYP_RING].mi_wght;
    when STICK:
	ws_know[wh] = TRUE;
	fix_stick(obj);
    when SCROLL:
	obj->o_weight = things[TYP_SCROLL].mi_wght;
	s_know[wh] = TRUE;
    when POTION:
	obj->o_weight = things[TYP_POTION].mi_wght;
	p_know[wh] = TRUE;
    }
    mpos = 0;
    if (add_pack(item, FALSE) == FALSE) {
	obj->o_pos = hero;
	fall(&player, item, TRUE);
    }
}

/*
 * getbless:
 *	Get a blessing for a wizards object
 */
getbless()
{
    register char reply;

    msg("Blessing? (+,-,n)");
    reply = readchar();
    switch (reply) {
    when '+':
	return (3);
    when '-':
	return (-3);
    otherwise:
	return (0);
    }
}

/*
 * make a monster for the wizard
 */
makemon() 
{
    register int i;
    register short which_monst;
    register int pres_monst=1, num_lines=LINES-3;
    char monst_name[2*LINELEN];

    /* Print out the monsters */
    while (pres_monst <= nummonst+2) {
	wclear(hw);
	touchwin(hw);

	wmove(hw, 2, 0);
	for (i=0; i<num_lines && pres_monst <= nummonst+2; i++) {
	    sprintf(monst_name, "[%d] %s\n",
				pres_monst, monsters[pres_monst].m_name);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	if (pres_monst <= nummonst+2) {
	    mvwaddstr(hw, LINES-1, 0, morestr);
	    wrefresh(hw);
	    wait_for(' ');
	}

	else {
	    mvwaddstr(hw, 0, 0, "Which monster");
	    if (!terse) waddstr(hw, " do you wish to create");
	    waddstr(hw, "? ");
	    wrefresh(hw);
	}
    }

    do {
	i = get_str(monst_name, hw);
	if (i == QUIT) {
	    restscr(cw);
	    msg("");
	    return;
	}
	which_monst = atoi(monst_name);
	if (which_monst < 1 || which_monst > nummonst+2) {
	    mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range: ");
	    wrefresh(hw);
	}
    } until (which_monst > 0 && which_monst <= nummonst+2);
    restscr(cw);
    creat_mons (&player, which_monst, TRUE);
    touchwin(cw);
}

/*
 * teleport:
 *	Bamf the hero someplace else
 */

teleport()
{
    register struct room *new_rp, *old_rp = roomin(&hero);
    register int rm, dir, which;
    coord c;
    bool is_lit = FALSE;	/* For saving room light state */
    bool rand_position = TRUE;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    if (is_wearing(R_TELCONTROL)) {
	msg("Where do you wish to teleport to? (* for help)");
	wmove(cw, hero.y, hero.x);
	wrefresh(cw);
	which = (short) (getchar() & 0177);
	while (which != (short)ESCAPE && which != (short)LINEFEED &&
					 which != (short)CARRIAGE_RETURN) {
	    switch(which) {
	    when 'h': c.x--;
	    when 'j': c.y++;
	    when 'k': c.y--;
	    when 'l': c.x++;
	    when 'y': c.x--; c.y--;
	    when 'u': c.x++; c.y--;
	    when 'b': c.x--; c.y++;
	    when 'n': c.x++; c.y++;
	    when '*':
		msg("Use h,j,k,l,y,u,b,n to position cursor, then hit return.");
	    }
	    c.y = max(c.y, 1);
	    c.y = min(c.y, LINES - 3);
	    c.x = max(c.x, 1);
	    c.x = min(c.x, COLS - 1);
	    wmove(cw, c.y, c.x);
	    wrefresh(cw);
	    which = (short) (getchar() & 0177);
	} 
	which = winat(c.y, c.x);
	if ((which == FLOOR || which == PASSAGE || which == DOOR) &&
	    ((ring_value(R_TELCONTROL) == 0 && rnd(10) < 6)
	    || (ring_value(R_TELCONTROL) > 0 && rnd(10) < 9))) {
	    rand_position = FALSE;
	    msg("You attempt succeeds.");
	    hero = c;
	    new_rp = roomin(&hero);
	}
	else msg("Your attempt fails.");
    }
    if (rand_position) {
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &hero);
	} until(winat(hero.y, hero.x) == FLOOR);
	new_rp = &rooms[rm];
    }

    /* If hero gets moved, darken old room */
    if (old_rp && old_rp != new_rp) {
	if (!(old_rp->r_flags & ISDARK)) is_lit = TRUE;
	old_rp->r_flags |= ISDARK;	/* Fake darkness */
	light(&c);
	if (is_lit) old_rp->r_flags &= ~ISDARK; /* Restore light state */
    }
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);

    /*
     * turn off ISHELD in case teleportation was done while fighting
     */
    if (on(player, ISHELD)) {
	register struct linked_list *ip, *nip;
	register struct thing *mp;

	turn_off(player, ISHELD);
	hold_count = 0;
	for (ip = mlist; ip; ip = nip) {
	    mp = THINGPTR(ip);
	    nip = next(ip);
	    if (on(*mp, DIDHOLD)) {
		turn_off(*mp, DIDHOLD);
		turn_on(*mp, CANHOLD);
	    }
	    turn_off(*mp, DIDSUFFOCATE);
	}
    }

    extinguish(suffocate);
    player.t_no_move = 0;			/* not trapped anymore */

    count = 0;
    running = FALSE;
    flushout();
    return rm;
}

/*
 * passwd:
 *	see if user knows password
 */

passwd()
{
    register char *sp, c;
    char buf[2*LINELEN], *crypt();

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = (getchar() & 0177)) != '\n' && c != '\r' && c != '\033')
#if (defined(USGV3) || defined(USGV5)) && !defined(SCO5)
	if (c == _tty.c_cc[VKILL])
#endif
#if defined(BSD4) || defined(RTU2) || defined(SCO5)
	if (c == _tty.sg_kill)
#endif
	    sp = buf;
#if (defined(USGV3) || defined(USGV5)) && !defined(SCO5)
	else if (c == _tty.c_cc[VERASE] && sp > buf)
#endif
#if defined(BSD4) || defined(RTU2) || defined(SCO5)
	else if (c == _tty.sg_erase && sp > buf)
#endif
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp(PASSWD, crypt(buf, PASSWDSEED)) == 0);
}

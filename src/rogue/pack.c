/*
 * Last Modified: 08/12/86
 */
#include <ctype.h>
#include "rogue.h"

#define ESCAPE_EXIT(x) if (x == ESCAPE) {after = FALSE; msg(""); return(NULL);}
#define BAD_NEWS	-1
#define BAD_LIST ((struct linked_list *) BAD_NEWS)
#define GOOD_NEWS	0

char type_list[] = "!?])/=:,";	/* things to inventory */

/*
 * Routines to deal with the pack. The new pack is implemented through the 
 * use of bags, and items are classed by their types (see rogue.h) which
 * also happen to be their display character.
 */

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of getting it off the ground.
 */
bool
add_pack(item, silent)
register struct linked_list *item;
bool silent;
{
    register struct linked_list *ip, *lp;
    register struct object *obj, *op;
    register bool exact, from_floor;
    register unsigned int index = 0;
  
    int bff_group();
    struct object *apply_to_bag();
  
    if (item == NULL) {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return(FALSE);
	debug("Picking up %s from floor.",
		name_type((OBJPTR(item))->o_type));
    }
    else
	from_floor = FALSE;
  
    obj = OBJPTR(item);
  
    /*
     * If it is gold, just add its value to rogue's purse and get rid
     * of it.
     */
  
    if (obj->o_type == GOLD) {
      register struct linked_list *mitem;
      register struct thing *tp;
      
      if (!silent) {
	  if (!terse) addmsg("You found ");
	  msg("%d gold pieces.", obj->o_count);
	}
      
      /* First make sure no greedy monster is after this gold.
       * If so, make the monster run after the rogue instead.
       */
      for (mitem = mlist; mitem != NULL; mitem = next(mitem)) {
	  tp = THINGPTR(mitem);
	  if (tp->t_dest == &obj->o_pos) tp->t_dest = &hero;
	}
      
	/*
	 * This will cause problems if people are able to drop
	 * and pick up gold, or when GOLDSTEAL monsters are killed.
	 */
	/* Thieves get EXP for gold they pick up */
	if (player.t_ctype == C_THIEF) {
	    pstats.s_exp += obj->o_count / 2;
	    check_level();
	}

      purse += obj->o_count;
      if (from_floor) {
	  detach(lvl_obj, item);
	  mvaddch(hero.y, hero.x,
		  (roomin(&hero) == NULL ? PASSAGE : FLOOR));
	  discard(item);
	}
      return(TRUE);
    }
  

  /*
   * see if he can carry any more weight
   */
  
  if (itemweight(obj) + pstats.s_pack > pstats.s_carry) {
      msg("Too much for you to carry.");
      if (!silent && levtype != POSTLEV) {
	  msg("%s onto %s", terse ? "Moved" : "You moved",
	      inv_name(obj, TRUE));
	}
      return FALSE;
    }
  
  /*
   * Link it into the pack.
   *  If the item can be grouped, try to find its neighbors and bump the
   * count. A special case is food, which can't be grouped, but an 
   * exact match allows the count to get incremented.
   */
  
  if ((op = apply_to_bag(pack, obj->o_type, bff_group, NULL, obj)) != NULL) {
      op->o_count += obj->o_count;	/* add it to the rest */
      if (from_floor) {
	  detach(lvl_obj, item);
	  mvaddch(hero.y, hero.x,
		  (roomin(&hero) == NULL ? PASSAGE : FLOOR));
	}
      pack_report(op, silent, "You now have ");
      discard(item);
      return(TRUE);
    }
  
  /*
   * Check for and deal with scare monster scrolls
   */
  
  if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
    if (obj->o_flags & ISCURSED) {
	msg("The scroll turns to dust as you pick it up.");
	detach(lvl_obj, item);
	discard(item);
	mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
	return(TRUE);
      }
  
  /*
   * Check if there is room
   */
  
  if (count_bag(pack, obj->o_type, NULL) == max_print()) {
      msg("You have no room for more %s.", name_type(obj->o_type));
      if (!silent && levtype != POSTLEV) {
	obj = OBJPTR(item);
	msg("%s onto %s.", terse ? "Moved" : "You moved",
	    inv_name(obj, TRUE));
      }
      return(FALSE);
    }
  
  /*
   * finally, add the new item to the bag, and free up the linked list on top
   * of it.
   */
  
  if (from_floor) {
      detach(lvl_obj, item);
      mvaddch(hero.y, hero.x,
	      (roomin(&hero) == NULL ? PASSAGE : FLOOR));
    }
  push_bag(&pack, obj, obj->o_type);
  pack_report(obj, silent, "You now have ");
  ur_free(item);
  return(TRUE);		/* signal success */
}

/*
 * pack_report
 * 
 * Notify the user about the results of the pack operation and do some 
 * post processing.
 */

pack_report(obj, silent, message)
register object *obj;
bool silent;
char *message;
{
  extern char print_letters[];

  /*
   * Notify the user
   */
  if (!silent) {
      if (!terse)
	addmsg(message);
      msg("(%c%c) %s.",obj->o_type, print_letters[get_ident(obj)],
	  inv_name(obj, !terse));
    }
  if (obj->o_flags & ISPOST)
    switch(obj->o_type) {
      when POTION:
        p_know[obj->o_which] = TRUE;
      when STICK:
        ws_know[obj->o_which] = TRUE;
      when SCROLL:
        s_know[obj->o_which] = TRUE;
      when RING:
        r_know[obj->o_which] = TRUE;
    }
  obj->o_flags &= ~ISPOST;
  if (obj->o_type == ARTIFACT) {
    has_artifact |= (1 << obj->o_which);
    picked_artifact |= (1 << obj->o_which);
    if ( !(obj->art_stats.ar_flags & ISUSED) ) {
      obj->art_stats.ar_flags |= ISUSED;
      pstats.s_exp += arts[obj->o_which].ar_worth / 10;
      check_level();
    }
  }
  debug("Object flags %x, picked up type %c, which %d.", 
	obj->o_flags, obj->o_type, obj->o_which);
  updpack(FALSE);
  return(TRUE);
}

/*
 * inventory:
 *	list what is in the pack
 */
inventory(container, type)
register char *container;	/* probably a bag */
register char type;		/* character for type, or NULL to prompt */
{
    register struct object *obj;
    int count;

    void baf_print_item();

    if (type == NULL) {
	msg("What kind of item <%s> to inventory (* for all)?", type_list);
	type = readchar();
	ESCAPE_EXIT(type);
    }

/*
 * Get a list of items to print out. If the user selects '*', list them
 * all.
 */
  
  if (type == '*')
    type = NULL;	/* no type passed ->use them all */

  mpos = 0;
  
  if ((count = count_bag(container, type, NULL)) == 0)
    msg("You don't have any %s.", name_type(type));
  else
    apply_to_bag(container, type, NULL, baf_print_item, type);
  end_line();
  if (count > 1)
    msg("");
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
     char ch;
{
  debug("picking up %c", ch);
  switch(ch)
    {
    default:
      debug("Where did you pick that up???");
    case GOLD:
    case ARMOR:
    case POTION:
    case FOOD:
    case WEAPON:
    case SCROLL:	
    case ARTIFACT:
    case RING:
    case STICK:
      add_pack(NULL, FALSE);
      break;
    }
}

/*
 * get_object:
 *
 * Pick something out of a pack for a purpose. The basic idea is to list all
 * the possibilities, let the user select one, get that item from the 
 * container, and pass it back to the calling routine.
 */
struct object *
get_object(container, purpose, type, bff_p)
     char *container;	/* what container has what we want */
     char *purpose;	/* a message (verb) to print if we cant find any */
     char type;		/* type (o_type) to pick out (NULL = any) */
     int (*bff_p)();	/* bag filter function to test item */
{
  register struct object *obj_p = NULL;
  register char sel_type, sel_id;	/* selected type and id */
  register char response;

  void baf_print_item();

  struct object *scan_bag();

  if (container == NULL)
    {
      msg("There isn't anything in there.");
      after = FALSE;
      return NULL;
    }
/*
 * Make sure we have at least one item that qualifies! 
 */
  if (apply_to_bag(container, type, bff_p, NULL, NULL) == NULL)
    {
      msg("You seem to have nothing to %s.",purpose);
      after = FALSE;
      return (NULL);
    }
 
  while (obj_p == NULL)
    {
      msg("What do you want to %s (* for list)?", purpose);
      response = readchar();
      debug("get_object: type=%c response=%c", type, response);
      ESCAPE_EXIT(response);
      if (response == '*')
	{
	  mpos = 0;
	  apply_to_bag(container, type, bff_p, baf_print_item, type);
	  end_line();
	  msg("What do you want to %s (* for list)?", purpose);
	  response = readchar();	/* want real response */
	}
      if (type == NULL)
	{
	  sel_type = response;
	  sel_id = readchar();
	  ESCAPE_EXIT(sel_id);
	}
      else
	{
	  sel_type = type;
	  if (is_member(type_list, response))
	    response = readchar();
	  ESCAPE_EXIT(response);
	  sel_id = response;
	}
      debug("get_object: sel_type=%c, sel_id=%c", sel_type, sel_id);
      if (NULL == (obj_p = scan_bag(container, sel_type, unprint_id(sel_id))))
	msg("couldn't find %c%c", sel_type, sel_id);
    }
  return(obj_p);
}

/*
 * get_item
 *
 * This is only an interim function that serves as an interface to the old
 * function get_item and its replacement get_object. It assumes a NULL
 * action routine and allocates a linked_list structure on top of
 * the object pointer.
 */

struct linked_list *
get_item(purpose, type)
     char *purpose;	/* a message (verb) to print if we cant find any */
     char type;		/* type (o_type) to pick out (NULL = any) */
{
register struct object *obj_p;

linked_list *make_item();

if ((obj_p =  get_object(pack, purpose, type, NULL)) == NULL)
  return(NULL);

return(make_item(obj_p));
}

/* 
 * del_pack:
 *	Take something out of the hero's pack and throw it away.
 */
del_pack(what)
     reg struct linked_list *what;
{
  rem_pack(OBJPTR(what));
  discard(what);
}

/*
 * discard_pack
 *
 * take an object from the pack and throw it away
 * (like del_pack, but without the linked_list structure)
 */

discard_pack(obj_p)

object *obj_p;
{
rem_pack(obj_p);
throw_away(obj_p);
}
/*
 * rem_pack
 *
 * Removes an item from the pack. 
 */

rem_pack(obj_p)

register struct object *obj_p;

{
  cur_null(obj_p);		/* check for current stuff */
  pop_bag(&pack, obj_p);
  updpack(FALSE);
  return(TRUE);			/* tell caller an item has been removed */
}
/*
 * cur_null:
 *	This updates cur_weapon etc for dropping things
 */
cur_null(op)
     reg struct object *op;
{
  if (op == cur_weapon)
    cur_weapon = NULL;
  else if (op == cur_armor)
    cur_armor = NULL;
  else if (op == cur_ring[LEFT_1])
    cur_ring[LEFT_1] = NULL;
  else if (op == cur_ring[LEFT_2])
    cur_ring[LEFT_2] = NULL;
  else if (op == cur_ring[LEFT_3])
    cur_ring[LEFT_3] = NULL;
  else if (op == cur_ring[LEFT_4])
    cur_ring[LEFT_4] = NULL;
  else if (op == cur_ring[RIGHT_1])
    cur_ring[RIGHT_1] = NULL;
  else if (op == cur_ring[RIGHT_2])
    cur_ring[RIGHT_2] = NULL;
  else if (op == cur_ring[RIGHT_3])
    cur_ring[RIGHT_3] = NULL;
  else if (op == cur_ring[RIGHT_4])
    cur_ring[RIGHT_4] = NULL;
}

/*
 * idenpack:
 *	Identify all the items in the pack
 */
idenpack()
{
  void baf_identify();

  apply_to_bag(pack, NULL, NULL, baf_identify, NULL);
}

/*
 * Print out the item on the floor.  Used by the move command.
 */
show_floor()
{
  register struct linked_list *item;
  register struct object *obj;
  
  item = find_obj(hero.y, hero.x);
  if (item != NULL) 
    {
      addmsg("%s onto ", terse ? "Moved" : "You moved");
      obj = OBJPTR(item);
      if (obj->o_type == GOLD)
	msg("%d gold pieces.", obj->o_count);
      else 
	{
	  addmsg(inv_name(obj, TRUE));
	  addmsg(".");
	  endmsg();
	}
    }
}

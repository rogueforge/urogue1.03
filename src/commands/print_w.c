/*
 * Last Modified: 05/25/86
 */
#include "rogue.h"

extern struct init_weps weaps[MAXWEAPONS];

main()
{
    register int i;

    printf("Weapon  Damage Throw  Weight Worth Flags\n");

    for (i=1; i<MAXWEAPONS; i++) {

	printf("%15.15s ",
	    weaps[i].w_name
	    );
	printf("%5s ",
	    weaps[i].w_dam
	    );
	printf(" %5s ",
	    weaps[i].w_hrl
	    );
	printf(" %5d ",
	    weaps[i].w_wght
	    );
	printf(" %5d ",
	    weaps[i].w_worth
	    );

	find_flags(i);

	if (weaps[i].w_launch != NONE)
	    printf(" by %s",
		weaps[weaps[i].w_launch].w_name
	    );

	printf("\n");
    }
}

find_flags(i)
register int i;
{
    register int k, flag, mask;
    if ( (flag=weaps[i].w_flags) != 0)
    {
	for (mask=01,k=0; k<16; k++,mask<<=1)
	switch (flag&mask) {
case ISCURSED:	printf(" CURSED");	break;
case ISKNOW:	printf(" KNOW");	break;
case ISPOST:	printf(" POST");	break;
case ISMETAL:	printf(" METAL");	break;
case ISPROT:	printf(" PROT");	break;
case ISBLESSED:	printf(" BLESS");	break;
case ISZAPPED:	printf(" ZAP");		break;
case ISVORPED:	printf(" VORPAL");	break;
case ISSILVER:	printf(" SILVER");	break;
case ISPOISON:	printf(" POISON");	break;
case CANRETURN:	printf(" MISS_RET");	break;
case ISOWNED:	printf(" ALWAYS_RET");	break;
case ISLOST:	printf(" DISSAPEARS");	break;
case ISMISL:	printf(" MISSLE");	break;
case ISMANY:	printf(" MANY");	break;
case CANBURN:	printf(" BURN");	break;
case ISSHARP:	printf(" SHARP");	break;
case ISBIG:	printf(" BIG");		break;
case ISLITTLE:	printf(" LITTLE");	break;
	}
    }
}

/*
 * Last Modified: 05/25/86
 */
#include "rogue.h"
#include <stdio.h>

/*
    char *mi_name;
    int mi_prob;
    int mi_worth;
    int mi_curse;
    int mi_bless;
*/

main()
{
    register int i;
    float q;

    printf("Scrolls   %%	Curse Bless Worth\n");
    for (i=0; i<MAXSCROLLS; i++) {

	printf("%15.15s  ",
	    s_magic[i].mi_name
	    );
	q = s_magic[i].mi_prob  / 10.0;
	printf("%-3.1f	",
	    q
	    );
	printf(" %2d ",
	    s_magic[i].mi_curse
	    );
	printf(" %3d ",
	    s_magic[i].mi_bless
	    );
	printf(" %5d ",
	    s_magic[i].mi_worth
	    );

	printf("\n");
    }

    printf("Potions   %%	Curse Bless Worth\n");
    for (i=0; i<MAXPOTIONS; i++) {

	printf("%15.15s  ",
	    p_magic[i].mi_name
	    );
	q = p_magic[i].mi_prob  / 10.0;
	printf("%-3.1f	",
	    q);
	printf(" %2d ",
	    p_magic[i].mi_curse
	    );
	printf(" %3d ",
	    p_magic[i].mi_bless
	    );
	printf(" %5d ",
	    p_magic[i].mi_worth
	    );

	printf("\n");
    }

    printf("Rings   %%	Curse Bless Worth\n");
    for (i=0; i<MAXRINGS; i++) {

	printf("%15.15s  ",
	    r_magic[i].mi_name
	    );
	q = r_magic[i].mi_prob  / 10.0;
	printf("%-3.1f	",
	    q
	    );
	printf(" %2d ",
	    r_magic[i].mi_curse
	    );
	printf(" %3d ",
	    r_magic[i].mi_bless
	    );
	printf(" %5d ",
	    r_magic[i].mi_worth
	    );

	printf("\n");
    }

    printf("Sticks   %%	Curse Bless Worth\n");
    for (i=0; i<MAXFOODS; i++) {

	printf("%15.15s  ",
	    fd_data[i].mi_name
	    );
	q = fd_data[i].mi_prob  / 10.0;
	printf("%-3.1f	",
	    q
	    );
	printf(" %2d ",
	    fd_data[i].mi_curse
	    );
	printf(" %3d ",
	    fd_data[i].mi_bless
	    );
	printf(" %5d ",
	    fd_data[i].mi_worth
	    );

	printf("\n");
    }

    printf("Food   %%	Curse Bless Worth\n");
    for (i=0; i<MAXSTICKS; i++) {

	printf("%15.15s  ",
	    ws_magic[i].mi_name
	    );
	q = ws_magic[i].mi_prob  / 10.0;
	printf("%-3.1f	",
	    q
	    );
	printf(" %2d ",
	    ws_magic[i].mi_curse
	    );
	printf(" %3d ",
	    ws_magic[i].mi_bless
	    );
	printf(" %5d ",
	    ws_magic[i].mi_worth
	    );

	printf("\n");
    }
}

#include <stdio.h>
#include <terminal.h>

//ArtOS Logo At Boot Screen
int artos_start() {
    kprintf("                                                                                ");
    kprintf("\n                                    #####                                     ");
    kprintf("\n                                 # # ### # #                                  ");
    kprintf("\n                                 ##   #   ##                                  ");
    kprintf("\n                                 ###  #  ###                                  ");
    kprintf("\n                                  # #   # #                                   ");
    kprintf("\n                                   ## # ##                                    ");
    kprintf("\n                                  ###   ###                                   ");
    kprintf("                                                                                ");
    kprintf("\n                                             #                                ");
    kprintf("\n                   #      #######   #######  # ####     ######                ");
    kprintf("\n                  # #     #      #     #     #     #   #                      ");
    kprintf("\n                 #####    #######      #     #      #   #####                 ");
    kprintf("\n                #     #   #     #      #     #     #         #                ");
    kprintf("\n               #       #  #      #     #     # ####    ######                 ");
    kprintf("\n                                             #                                ");
    kprintf("\n                                                                              ");
    
    return 0;
}

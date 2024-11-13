/**
 * ===================================================================
 * @brief VERY IMPORTANT MACROS !!!
 * DO NOT CHANGE THE THINGS BELOW WITHOUT KNOWING WHAT YOU ARE DOING
 * IT AFFECTS ACROSS THE WHOLE CODEBASE. YOU HAVE BEEN WARNED !!!
 * ===================================================================
 */
#define IS_GWY true
#define CLIENT_RELEASE false
/*====================================================================*/

#if(IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 8
#define PATCH_VERSION 9
#endif

#if(!IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 8
#define PATCH_VERSION 9
#endif


/*Global Variables*/
extern bool mqtt_connected;
extern bool registered;
extern bool provisioned;
extern bool configured;
extern bool sending_ir_command;
extern bool teaching_in_progress;
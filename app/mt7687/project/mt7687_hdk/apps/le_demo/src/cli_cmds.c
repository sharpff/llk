

#include <stdint.h>


#if defined(MTK_MINICLI_ENABLE)


#include "minicli_cmd_table.h"


static cli_t *_cli_ptr;


#ifdef MTK_CLI_TEST_MODE_ENABLE
static uint8_t _sdk_cli_test_mode(uint8_t len, char *param[]);
#endif


#ifdef MTK_CLI_EXAMPLE_MODE_ENABLE
static uint8_t _sdk_cli_example_mode(uint8_t len, char *param[]);
#endif


#if defined(MTK_CLI_TEST_MODE_ENABLE) || defined(MTK_CLI_EXAMPLE_MODE_ENABLE)
static uint8_t _sdk_cli_normal_mode(uint8_t len, char *param[]);
#endif

#ifdef MTK_CLI_TEST_MODE_ENABLE
#define GOTO_TEST_MODE_CLI_ENTRY    { "en",   "enter test mode",     _sdk_cli_test_mode    },
#endif

#ifdef MTK_CLI_EXAMPLE_MODE_ENABLE
#define GOTO_EXAMPLE_MODE_CLI_ENTRY { "ex",   "enter example mode",  _sdk_cli_example_mode },
#else
#define GOTO_EXAMPLE_MODE_CLI_ENTRY
#endif

#if defined(MTK_CLI_TEST_MODE_ENABLE) || defined(MTK_CLI_EXAMPLE_MODE_ENABLE)
#define GOTO_NORMAL_MODE_CLI_ENTRY  { "back", "back to normal mode", _sdk_cli_normal_mode  },
#endif


#ifndef GOTO_TEST_MODE_CLI_ENTRY
#define GOTO_TEST_MODE_CLI_ENTRY
#endif

#ifndef GOTO_EXAMPLE_MODE_CLI_ENTRY
#define GOTO_EXAMPLE_MODE_CLI_ENTRY
#endif

#ifndef GOTO_NORMAL_MODE_CLI_ENTRY
#define GOTO_NORMAL_MODE_CLI_ENTRY
#endif

extern uint8_t le_reboot(uint8_t len, char *param[]);
extern uint8_t le_version(uint8_t len, char *param[]);
extern uint8_t le_ledon(uint8_t len, char *param[]);
extern uint8_t le_ledoff(uint8_t len, char *param[]);
extern uint8_t le_ledset(uint8_t len, char *param[]);
extern uint8_t le_ledsetRGB(uint8_t len, char *param[]);
extern uint8_t le_resetPID(uint8_t len, char *param[]);

cmd_t le_cmd[] = {
    { "version", "software download", le_version },
    { "ledon", "led on", le_ledon },
    { "ledoff", "led off", le_ledoff },
    { "ledset", "led set", le_ledset },
    { "ledsetRGB", "led set RGB", le_ledsetRGB },
    { "resetPID", "reset PID", le_resetPID },
    { "reboot", "hardware reboot", le_reboot },
    { NULL }
};

#define GOTO_LELINK_CMD          { "le",    "lelink command",     NULL,   le_cmd },

/****************************************************************************
 *
 * EXAMPLE MODE
 *
 ****************************************************************************/


#ifdef MTK_CLI_EXAMPLE_MODE_ENABLE

static cmd_t   _cmds_example[] = {
    GOTO_NORMAL_MODE_CLI_ENTRY
    MINICLI_EXAMPLE_MODE_CLI_CMDS
    { NULL, NULL, NULL, NULL }
};

#endif /* MTK_CLI_EXAMPLE_MODE_ENABLE */


/****************************************************************************
 *
 * TEST MODE
 *
 ****************************************************************************/


#ifdef MTK_CLI_TEST_MODE_ENABLE

static cmd_t   _cmds_test[] = {
    GOTO_NORMAL_MODE_CLI_ENTRY
    MINICLI_TEST_MODE_CLI_CMDS
    { NULL, NULL, NULL, NULL }
};

#endif /* MTK_CLI_TEST_MODE_ENABLE */


/****************************************************************************
 *
 * NORMAL MODE
 *
 ****************************************************************************/


static cmd_t   _cmds_normal[] = {
    GOTO_TEST_MODE_CLI_ENTRY
    GOTO_EXAMPLE_MODE_CLI_ENTRY
    MINICLI_NORMAL_MODE_CLI_CMDS
    OS_CLI_ENTRY
    GOTO_LELINK_CMD
    { NULL, NULL, NULL, NULL }
};


/****************************************************************************
 *
 * TOGGLE commands
 *
 ****************************************************************************/


#ifdef MTK_CLI_TEST_MODE_ENABLE
static uint8_t _sdk_cli_test_mode(uint8_t len, char *param[])
{
    _cli_ptr->cmd = &_cmds_test[0];
    return 0;
}
#endif


#ifdef MTK_CLI_EXAMPLE_MODE_ENABLE
static uint8_t _sdk_cli_example_mode(uint8_t len, char *param[])
{
    _cli_ptr->cmd = &_cmds_example[0];
    return 0;
}
#endif


#if defined(MTK_CLI_TEST_MODE_ENABLE) || defined(MTK_CLI_EXAMPLE_MODE_ENABLE)
static uint8_t _sdk_cli_normal_mode(uint8_t len, char *param[])
{
    _cli_ptr->cmd = &_cmds_normal[0];
    return 0;
}
#endif


/****************************************************************************
 *
 * PUBLIC functions
 *
 ****************************************************************************/


void cli_cmds_init(cli_t *cli)
{
    _cli_ptr = cli;
    _cli_ptr->cmd = &_cmds_normal[0];
}


#endif /* #if defined(MTK_MINICLI_ENABLE) */

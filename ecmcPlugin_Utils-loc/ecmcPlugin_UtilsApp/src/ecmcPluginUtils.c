/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPluginExample.cpp
*
*  Created on: Mar 21, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN
#define ECMC_EXAMPLE_PLUGIN_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif  // ifdef __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecmcPluginDefs.h"
#include "ecmcPvaWrap.h"
#include "ecmcPluginClient.h"
#include "ecmcPvDefs.h"

// only allow to load once
#define ECMC_PLUGIN_ALLOW_MULTI_LOAD 0

// Error codes
#define ECMC_PLUGIN_ERROR_ALREADY_LOADED 1

static int    lastEcmcError = 0;
static char*  lastConfStr   = NULL;
static int    loaded = 0;

extern struct ecmcPluginData pluginDataDef;

/** Optional. 
 *  Will be called once after successfull load into ecmc.
 *  Return value other than 0 will be considered error.
 *  configStr can be used for configuration parameters.
 **/
int pvaConstruct(char *configStr)
{
  //This module is only allowed to load once
  
  if(loaded && !ECMC_PLUGIN_ALLOW_MULTI_LOAD) {
    printf("%s/%s:%d: Error: Module already loaded (0x%x).\n",__FILE__, __FUNCTION__,
           __LINE__,ECMC_PLUGIN_ERROR_ALREADY_LOADED);
    return ECMC_PLUGIN_ERROR_ALREADY_LOADED;
  }
  // create Pva object and register data callback
  lastConfStr = strdup(configStr);

  // Add refs to generic funcs in runtime since objects
  pluginDataDef.funcs[0].funcGenericObj = getPvRegObj();  
  loaded = 1;
  return 0;
}

/** Optional function.
 *  Will be called once at unload.
 **/
void pvaDestruct(void)
{
  if(lastConfStr){
    free(lastConfStr);
  }
}

/** Optional function.
 *  Will be called each realtime cycle if definded
 *  ecmcError: Error code of ecmc. Makes it posible for 
 *  this plugin to react on ecmc erro-I/epics/base-7.0.3.1/include/os/Linuxrs
 *  Return value other than 0 will be considered to be an error code in ecmc.
 **/
int pvaRealtime(int ecmcError)
{ 
  lastEcmcError = ecmcError;
  return 0;
}

/** Link to data source here since all sources should be availabe at this stage
 *  (for example ecmc PLC variables are defined only at enter of realtime)
 **/
int pvaEnterRT(){
  return 0;
}

/** Optional function.
 *  Will be called once just before leaving realtime mode
 *  Return value other than 0 will be considered error.
 **/
int pvaExitRT(void){
  return 0;
}

// Normal PLC functions
double pvaExeGetCmd(double handle) {
  return (double)exeGetDataCmd((int)handle);
}

double pvaExePutCmd(double handle, double value) {
  return (double)exePutDataCmd((int)handle, value);
}

double pvaGetLastValue(double handle) {
  return getLastValue((int)handle);
}

double pvaGetBusy(double handle) {
  return getBusy((int)handle);
}

double pvaGetErr(double handle) {
  return (double)getError((int)handle);
}

double pvaRstErr(double handle) {
  resetError((int)handle);
  return 0.0;
}

double pvaGetIOCStarted() {
  return (double)(getEcmcEpicsIOCState()==16);
}

double pvaGetIOCState() {
  return (double)getEcmcEpicsIOCState();
}

// Register data for plugin so ecmc know what to use
struct ecmcPluginData pluginDataDef = {
  // Allways use ECMC_PLUG_VERSION_MAGIC
  .ifVersion = ECMC_PLUG_VERSION_MAGIC, 
  // Name 
  .name = "ecmcPlugin_Utils",
  // Description
  .desc = "Utility plugin for use with ecmc. Funcs: pvAccess, ioc status.",
  // Option description
  .optionDesc = "No options",
  // Plugin version
  .version = ECMC_EXAMPLE_PLUGIN_VERSION,
  // Optional construct func, called once at load. NULL if not definded.
  .constructFnc = pvaConstruct,
  // Optional destruct func, called once at unload. NULL if not definded.
  .destructFnc = pvaDestruct,
  // Optional func that will be called each rt cycle. NULL if not definded.
  .realtimeFnc = pvaRealtime,
  // Optional func that will be called once just before enter realtime mode
  .realtimeEnterFnc = pvaEnterRT,
  // Optional func that will be called once just before exit realtime mode
  .realtimeExitFnc = pvaExitRT,
  // PLC funcs
  .funcs[0] =
      { /*----pv_reg_async----*/
        .funcName = ECMC_PV_PLC_CMD_PV_REG_ASYNC,
        .funcDesc = "handle = " ECMC_PV_PLC_CMD_PV_REG_ASYNC "(<pv name>, <provider name pva/ca>) : register new pv.",
        .funcArg0 = NULL,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,        
        .funcGenericObj = NULL,  //will be assigned here during plugin construct (cannot initiate with non-const)
      },
  .funcs[1] =
      { /*----pv_put_async----*/
        .funcName = ECMC_PV_PLC_CMD_PV_PUT_ASYNC,
        .funcDesc = "error = " ECMC_PV_PLC_CMD_PV_PUT_ASYNC "(<handle>, <value>) : Execute async pv_put cmd.",
        .funcArg0 = NULL,
        .funcArg1 = NULL,
        .funcArg2 = pvaExePutCmd,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[2] =
      { /*----pv_get_async----*/
        .funcName = ECMC_PV_PLC_CMD_PV_GET_ASYNC,
        .funcDesc = "error = " ECMC_PV_PLC_CMD_PV_GET_ASYNC "(<handle>) : Execute async pv_get cmd.",
        .funcArg0 = NULL,
        .funcArg1 = pvaExeGetCmd,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[3] =
      { /*----pv_get_value----*/
        .funcName = ECMC_PV_PLC_CMD_PV_GET_VALUE,
        .funcDesc = "value = " ECMC_PV_PLC_CMD_PV_GET_VALUE "(<handle>) : Get result from last pv_get_async() cmd.",
        .funcArg0 = NULL,
        .funcArg1 = pvaGetLastValue,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[4] =
      { /*----pv_get_busy----*/
        .funcName = ECMC_PV_PLC_CMD_PV_GET_BUSY,
        .funcDesc = "busy = "  ECMC_PV_PLC_CMD_PV_GET_BUSY "(<handle>) : Get status of last async command (pv_reg_async(), pv_get_async(), pv_put_async()).",
        .funcArg0 = NULL,
        .funcArg1 = pvaGetBusy,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },

  .funcs[5] =
      { /*----pv_get_err----*/
        .funcName = ECMC_PV_PLC_CMD_PV_GET_ERR,
        .funcDesc = "error = " ECMC_PV_PLC_CMD_PV_GET_ERR "(<handle>) : Get error code.",
        .funcArg0 = NULL,
        .funcArg1 = pvaGetErr,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[6] =
      { /*----pv_rst_err----*/
        .funcName = ECMC_PV_PLC_CMD_PV_RST_ERR,
        .funcDesc = ECMC_PV_PLC_CMD_PV_RST_ERR "(<handle>) : Reset error code.",
        .funcArg0 = NULL,
        .funcArg1 = pvaRstErr,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[7] =
      { /*----ut_get_ecmc_ioc_state----*/
        .funcName = "ioc_get_state",
        .funcDesc = "state = ioc_get_state() : Get ecmc epics ioc state.",
        .funcArg0 = pvaGetIOCState,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[8] =
      { /*----ut_get_ecmc_ioc_started----*/
        .funcName = "ioc_get_started",
        .funcDesc = "started = ioc_get_started() : Get ecmc epics ioc started.",
        .funcArg0 = pvaGetIOCStarted,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },

  .funcs[9]  = {0}, // last element set all to zero..
  .consts[0] = {0}, // last element set all to zero..
};

ecmc_plugin_register(pluginDataDef);

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus

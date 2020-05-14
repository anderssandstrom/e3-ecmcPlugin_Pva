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
double pvaGet(double handle) {
  return getData(handle);
}

double pvaPut(double handle, double value) {
  return putData(handle, value);
}

double pvaGetErr(double handle) {
  return getError(handle);
}

double pvaRstErr(double handle) {
  reset(handle);
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
  .name = "ecmcPlugin_Pva",
  // Description
  .desc = "pvAccess plugin for use with ecmc.",
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
      { /*----pv_reg----*/
        .funcName = "pv_reg",
        .funcDesc = "handle = pv_reg(<pv name>, <provider name pva/ca>) : register new pv.",
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
        .funcGenericObj = NULL,  // will assigned here during construct
      },
  .funcs[1] =
      { /*----pv_put----*/
        .funcName = "pv_put",
        .funcDesc = "error = pv_put(<handle>, <value>) : Set pv value.",
        .funcArg0 = NULL,
        .funcArg1 = NULL,
        .funcArg2 = pvaPut,
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
      { /*----pv_get----*/
        .funcName = "pv_get",
        .funcDesc = "value = pv_get(<handle>) : Get pv value.",
        .funcArg0 = NULL,
        .funcArg1 = pvaGet,
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
      { /*----pv_get_err----*/
        .funcName = "pv_get_err",
        .funcDesc = "error = pv_get_err(<handle>) : Get error code.",
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
  .funcs[4] =
      { /*----pv_rst_err----*/
        .funcName = "pv_rst_err",
        .funcDesc = "pv_rst_err(<handle>) : Reset error code.",
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
  .funcs[5] =
      { /*----get_ecmc_ioc_state----*/
        .funcName = "get_ecmc_ioc_state",
        .funcDesc = "state = get_ecmc_ioc_state() : Get ecmc epics ioc state.",
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
  .funcs[6] =
      { /*----get_ecmc_ioc_started----*/
        .funcName = "get_ecmc_ioc_started",
        .funcDesc = "started = get_ecmc_ioc_started() : Get ecmc epics ioc started.",
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

  .funcs[7]  = {0}, // last element set all to zero..
  .consts[0] = {0}, // last element set all to zero..
};

ecmc_plugin_register(pluginDataDef);

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus

e3-ecmcPlugin_Utils
======
ESS Site-specific EPICS module : ecmcPlugin_Utils

A shared library utility functions loadable into ecmc.

## PvAccess 
Implements functions for accessing pv:s over pvAccess from ecmc plc:s.

### Registering and writing pvs:
Registration and writes are implementad as async commands in order to minimize blocking time of ecmc realtime thread. Even though the "pvaClient::issue*" commands are non blocking they were idetified to consume to much time. Therefore both registration and writing commands are handled async by a low prio worker thread. The pv_busy() command will return high as long as the worker thread is procssing and low when done (see examples in "iocsh" dir).

### Reading values:
A monitor is continiously updating the current value of the pv and making it accessible to read from a ecmc by "pv_get()" command in an ecmc-plc.

### PLC-functions:
  * handle = pv_reg_async( pvName, provider ) : Exe. async cmd to register PV. Returns handle to PV-object or error (if < 0). Provider needs to be set to either "pva" or "ca" (ca to be able to access pv:s in EPICS 3.* IOC:s).  
  * error  = pv_put_async( handle, value ) : Exe async pv put command.  Retruns error-code.
  * value  = pv_get( handle ): Get pv value from last monitor update.
  * busy   = pv_busy( handle ) : Return if PV-object is busy (busy if a pv_put_asyn() or a pv_reg_asyn() async command is executing).
  * error  = pv_err( handle ) : Returns error code of PV-objects last command (error > 0).
  * connected = pv_connected(<handle>) : Return if pv is connected.

### Config options
MAX_PV_COUNT=<count> : Sets the maximum number of pv:s to register. These pv objects will be allocated when module is loaded (before realtime). This setting defaults to 8.

### Record support
The functions currently only support scalar values. Value field of following record types have been tested:
* AI
* AO
* BI (enum_t: return index of enum value)
* BO (enum_t: sets index of enum value)

### Example
The example in the "iocsh" dir shows how to use the pva functions. The example demonstartes how a ecmc plc (in an ecmc ioc) can connect to an external ioc. The ecmc plc registers, writes and reads values from the folowing records:
* IOC_DUMMY:AI
* IOC_DUMMY:AO
* IOC_DUMMY:BI
* IOC_DUMMY:BO

Start the "ecmc-ioc" (from iocsh dir):
```
$ iocsh.bash ecmc_ioc.script
```
Start the "external-ioc" (from iocsh dir):
```
$ iocsh.bash external_ioc.script
```

The "ecmc-ioc" will now connect to, read and write pvs in the "external-ioc" and generates some printouts:
```
Get AI from PV: 117.00000
Put AO to PV  : 118.00000
pv_put_asyn AO exe time [ns] : 10240.00000
Busy after pv_put_asyn():    1.00000
Get BI from PV:   1.00000
Put BO to PV  :   0.00000
Busy after pv_put_asyn():    1.00000, err:    0.00000
pv_get AI exe time [ns] : 4096.00000
Get AI from PV: 118.00000
Put AO to PV  : 119.00000
pv_put_asyn AO exe time [ns] : 10240.00000
Busy after pv_put_asyn():    1.00000
Get BI from PV:   0.00000
Put BO to PV  :   1.00000

```

## EPICS utils:
  * started = ioc_get_started() : ecmc IOC up and running
  * state = ioc_get_state()   : ecmc IOC state (hook)

## Setup
```
$ make install
```

## Plugin Info
```
Plugin info: 
  Index                = 0
  Name                 = ecmcPlugin_Utils
  Description          = Utility plugin for use with ecmc. Funcs: pvAccess, ioc status.
  Option description   = MAX_PV_COUNT=<count> : Set max number of pvs to connect to (defaults to 8).
  Filename             = /epics/base-7.0.3.1/require/3.1.2/siteMods/ecmcPlugin_Utils/master/lib/linux-arm/libecmcPlugin_Utils.so
  Config string        = MAX_PV_COUNT=5
  Version              = 2
  Interface version    = 65536 (ecmc = 65536)
     max plc funcs     = 64
     max plc func args = 10
     max plc consts    = 64
  Construct func       = @0xb5023b4c
  Enter realtime func  = @0xb5023a58
  Exit realtime func   = @0xb5023a60
  Realtime func        = @0xb5023a50
  Destruct func        = @0xb5023a68
  dlhandle             = @0x1844100
  Plc functions:
    funcs[00]:
      Name       = "pv_reg_asyn();"
      Desc       = handle = pv_reg_asyn(<pv name>, <provider name pva/ca>) : register new pv.
      func       = @0x18581d8
    funcs[01]:
      Name       = "pv_put_asyn(arg0, arg1);"
      Desc       = error = pv_put_asyn(<handle>, <value>) : Execute async pv_put cmd.
      Arg count  = 2
      func       = @0xb5023a8c
    funcs[02]:
      Name       = "pv_get(arg0);"
      Desc       = value = pv_get(<handle>) : Get value of registered pv (updated by monitor).
      Arg count  = 1
      func       = @0xb5023aac
    funcs[03]:
      Name       = "pv_busy(arg0);"
      Desc       = busy = pv_busy(<handle>) : Get status of last async command (pv_reg_asyn(), pv_put_asyn()).
      Arg count  = 1
      func       = @0xb5023ab8
    funcs[04]:
      Name       = "pv_err(arg0);"
      Desc       = error = pv_err(<handle>) : Get error code.
      Arg count  = 1
      func       = @0xb5023af0
    funcs[05]:
      Name       = "pv_connected(arg0);"
      Desc       = connected = pv_connected(<handle>) : Get pv connected.
      Arg count  = 1
      func       = @0xb5023ad4
    funcs[06]:
      Name       = "ioc_get_state();"
      Desc       = state = ioc_get_state() : Get ecmc epics ioc state.
      Arg count  = 0
      func       = @0xb5023b38
    funcs[07]:
      Name       = "ioc_get_started();"
      Desc       = started = ioc_get_started() : Get ecmc epics ioc started.
      Arg count  = 0
      func       = @0xb5023b0c
  Plc constants:

```

e3-ecmcPlugin_Utils
======
ESS Site-specific EPICS module : ecmcPlugin_Utils

A shared library utility functions loadable into ecmc:
* PvAccess 
  * handle = pv_reg_async( pvName, provider ) : Exe. async cmd to register PV. Returns handle to PV-object or error (if < 0). Provider needs to be set to either "pva" or "ca" (ca to be able to access pv:s in EPICS 3.* IOC:s).
  * error  = pv_get_async( handle ) : Exe asyncronous pv get command. Retruns error-code.
  * error  = pv_put_async( handle, value ) : Exe asyncronous pv put command.  Retruns error-code.
  * value  = pv_value( handle ): Get value from last successful read or write command
  * busy   = pv_busy( handle ) : Return if PV-object is busy (busy if a get, put or a reg commnd is executing).
  * error  = pv_err( handle ) : Returns error code of PV-object (error > 0).
  * pv_rst( handle ) : Reset error of PV-object.
* EPICS State:
  * started = ioc_get_started() : ecmc IOC up and running
  * state = ioc_get_state()   : ecmc IOC state (hook)




## Plugin Info
```
Plugin info: 
  Index                = 0
  Name                 = ecmcPlugin_Utils
  Description          = Utility plugin for use with ecmc. Funcs: pvAccess, ioc status.
  Option description   = No options
  Filename             = /epics/base-7.0.3.1/require/3.1.2/siteMods/ecmcPlugin_Utils/master/lib/linux-x86_64/libecmcPlugin_Utils.so
  Config string        = EMPTY
  Version              = 2
  Interface version    = 512 (ecmc = 512)
     max plc funcs     = 64
     max plc func args = 10
     max plc consts    = 64
  Construct func       = @0x7f0e52caeef0
  Enter realtime func  = @0x7f0e52caedb0
  Exit realtime func   = @0x7f0e52caedc0
  Realtime func        = @0x7f0e52caeda0
  Destruct func        = @0x7f0e52caedd0
  dlhandle             = @0x15132b0
  Plc functions:
    funcs[00]:
      Name       = "pv_reg_async();"
      Desc       = handle = pv_reg_async(<pv name>, <provider name pva/ca>) : register new pv.
      func       = @0x1517a20
    funcs[01]:
      Name       = "pv_put_async(arg0, arg1);"
      Desc       = error = pv_put_async(<handle>, <value>) : Execute async pv_put cmd.
      Arg count  = 2
      func       = @0x7f0e52caee10
    funcs[02]:
      Name       = "pv_get_async(arg0);"
      Desc       = error = pv_get_async(<handle>) : Execute async pv_get cmd.
      Arg count  = 1
      func       = @0x7f0e52caedf0
    funcs[03]:
      Name       = "pv_value(arg0);"
      Desc       = value = pv_value(<handle>) : Get result from last pv_get_async() or pv_put_async() cmd.
      Arg count  = 1
      func       = @0x7f0e52caee30
    funcs[04]:
      Name       = "pv_busy(arg0);"
      Desc       = busy = pv_busy(<handle>) : Get status of last async command (pv_reg_async(), pv_get_async(), pv_put_async()).
      Arg count  = 1
      func       = @0x7f0e52caee40
    funcs[05]:
      Name       = "pv_err(arg0);"
      Desc       = error = pv_err(<handle>) : Get error code.
      Arg count  = 1
      func       = @0x7f0e52caee60
    funcs[06]:
      Name       = "pv_rst(arg0);"
      Desc       = pv_rst(<handle>) : Reset error code.
      Arg count  = 1
      func       = @0x7f0e52caee80
    funcs[07]:
      Name       = "ioc_get_state();"
      Desc       = state = ioc_get_state() : Get ecmc epics ioc state.
      Arg count  = 0
      func       = @0x7f0e52caeed0
    funcs[08]:
      Name       = "ioc_get_started();"
      Desc       = started = ioc_get_started() : Get ecmc epics ioc started.
      Arg count  = 0
      func       = @0x7f0e52caeea0
```

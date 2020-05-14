e3-ecmcPlugin_Utils
======
ESS Site-specific EPICS module : ecmcPlugin_Utils

A shared library utility functions loadable into ecmc:
* PvAccess 
  * pv_reg()  : Register PV
  * pv_get()  : Get value from PV
  * pv_put()  : Put value to PV
* EPICS State:
  * get_ecmc_ioc_started() : IOC up and running
  * get_ecmc_ioc_state()   : IOC state (hook)

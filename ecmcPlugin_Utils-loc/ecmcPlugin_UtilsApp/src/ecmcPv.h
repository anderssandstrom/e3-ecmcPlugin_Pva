/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPv.h
*
*  Created on: May 12, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

#ifndef ECMC_PV_H_
#define ECMC_PV_H_

#include <string>
#include "pv/pvaClient.h"
#include <pv/convert.h>
#include "ecmcPvDefs.h"
#include "ecmcPluginClient.h"

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

class ecmcPv {
 public:
  ecmcPv(std::string name, std::string providerName);
  ~ecmcPv();
  int    getError();
  int    reset();
  double get();
  void   put(double value);

 private:
  std::string           name_;
  std::string           providerName_;
//   pvac::ClientProvider *provider_;
//   pvac::ClientChannel  *channel_;
  int                   errorCode_;  
  
  // General
  PvaClientPtr pva_;
  PvaClientChannelPtr pvaChannel_;    
  
  // Get
  PvaClientGetPtr get_;
  PvaClientGetDataPtr getData_;

  // Put
  PvaClientPutPtr put_;
  PvaClientPutDataPtr putData_;

  // Monitor
  PvaClientMonitorPtr monitor_;
  PvaClientMonitorDataPtr monitorData_;

};

#endif  /* ECMC_PV_H_ */

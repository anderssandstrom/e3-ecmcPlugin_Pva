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
#include <atomic>         // std::atomic_flag
#include "pv/pvaClient.h"
#include <pv/convert.h>
#include "ecmcPvDefs.h"
#include "ecmcPluginClient.h"
#include "epicsThread.h"
#include "epicsMutex.h"

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

enum ecmc_pva_cmd {
  ECMC_PV_CMD_NONE = 0,
  ECMC_PV_CMD_GET  = 1,
  ECMC_PV_CMD_PUT  = 2,  
  ECMC_PV_CMD_REG  = 3,  
};

 class ecmcPv;
 typedef std::tr1::shared_ptr<ecmcPv> ecmcPvPtr;


class ecmcPv :  public PvaClientChannelStateChangeRequester,
                public std::tr1::enable_shared_from_this<ecmcPv>
{
 public:
  POINTER_DEFINITIONS(ecmcPv);
  ecmcPv(std::string name, std::string providerName, int index);
  ~ecmcPv();
  int    getError();
  int    reset();
  virtual void   channelStateChange(PvaClientChannelPtr const & channel, bool isConnected);
  // Async Commads
  void   getCmd();
  void   putCmd(double value);
  void   regCmd();

  double getLastReadValue();
  bool   busy();
  bool   connected();
  void   exeCmdThread();
  void   monitorThread();
  std::string getPvName();
  std::string getProvider();
  static ecmcPvPtr create(std::string pvName,std::string providerName, int index);
 private:
 int    validateType();
 double getDouble();
 void   putDouble(double value);
 static std::string    to_string(int value);
 int    connect();

  std::string           channelName_;
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
  
  Type                  type_;
  // Thread related
  epicsEvent            doCmdEvent_;
  int                   destructs_;
  ecmc_pva_cmd          cmd_;
  std::atomic_flag      busyLock_;  

  int                   index_;
  int                   reconnectCounter_;
  double                valueLatestRead_;
  double                valueToWrite_;
  bool                  connected_;
  epicsThreadId         cmdExeThread_;
  epicsThreadId         monThread_;
  epicsMutexId          ecmcGetValMutex_;
  
};

#endif  /* ECMC_PV_H_ */

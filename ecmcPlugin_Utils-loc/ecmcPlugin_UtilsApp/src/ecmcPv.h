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

#include <iostream>
#include <epicsGetopt.h>
// std::atomic_flag
#include "pv/pvaClient.h"
#include "ecmcPvDefs.h"
#include "ecmcPluginClient.h"
#include <atomic>  
//#include "epicsThread.h"
//#include "epicsMutex.h"

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
                public PvaClientMonitorRequester,
                public std::tr1::enable_shared_from_this<ecmcPv>
{
 public:
  POINTER_DEFINITIONS(ecmcPv);
  ecmcPv(const std::string &channelName,
         const std::string &providerName,
         const std::string &request, 
         int index);
  ~ecmcPv();

  static ecmcPvPtr create(PvaClientPtr const & pvaClient,
                          const std::string  & channelName, 
                          const std::string  & providerName,
                          const std::string  & request,
                          int index);
  void   init(PvaClientPtr const &pvaClient);
  PvaClientMonitorPtr getPvaClientMonitor();
  int    getError();
  int    reset();
  // Async Commads
  // void   getCmd();
  // void   putCmd(double value);
  // void   regCmd();

  double getLastReadValue();
  bool   busy();
  bool   connected();
  // void   exeCmdThread();
  // void   monitorThread();
  std::string getChannelName();
  std::string getProvider();
  static ecmcPvPtr create(std::string pvName,std::string providerName, int index);
  virtual void monitorConnect(epics::pvData::Status const & status,
                              PvaClientMonitorPtr const & monitor,
                              epics::pvData::StructureConstPtr const & structure);
  virtual void event(PvaClientMonitorPtr const & monitor);
  virtual void channelStateChange(PvaClientChannelPtr const & channel,
                                  bool isConnected);

 private:
//  int    validateType();
//  double getDouble();
//  void   putDouble(double value);
//  static std::string    to_string(int value);
//  int    connect();

  std::string  channelName_;
  std::string  providerName_;
  std::string  request_;
//   pvac::ClientProvider *provider_;
//   pvac::ClientChannel  *channel_;
  bool channelConnected_;
  bool monitorConnected_;
  bool isStarted_;
  int                   index_;
  int                   errorCode_;  
  double                valueLatestRead_;
  double                valueToWrite_;  
  Type                  type_;
  // Thread related
  //epicsEvent            doCmdEvent_;
  //int                   destructs_;
  //ecmc_pva_cmd          cmd_;
  std::atomic_flag      busyLock_;  

  //int                   reconnectCounter_;
  
  // General
  PvaClientPtr pva_;
  PvaClientChannelPtr pvaClientChannel_;    
  
  // Get
  PvaClientGetPtr get_;
  PvaClientGetDataPtr getData_;

  // Put
  PvaClientPutPtr put_;
  PvaClientPutDataPtr putData_;

  // Monitor       
  PvaClientMonitorPtr pvaClientMonitor_;
  //PvaClientMonitorDataPtr monitorData_;
  
  //epicsThreadId         cmdExeThread_;
  //epicsThreadId         monThread_;
  //epicsMutexId          ecmcGetValMutex_;
  
};

#endif  /* ECMC_PV_H_ */

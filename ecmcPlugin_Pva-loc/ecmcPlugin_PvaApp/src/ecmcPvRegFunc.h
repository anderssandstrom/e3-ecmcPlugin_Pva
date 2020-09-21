/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPva.cpp
*
*  Created on: May 12, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

#include "pva/client.h"
#include "ecmcPv.h"
#include "ecmcPvDefs.h"
#include "exprtk.hpp"
#include "ecmcPluginClient.h"

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

typedef std::tr1::shared_ptr<ecmcPv> ecmcPvPtr;
vector<ecmcPvPtr> pvVector;

// class for exprtk handle=pv_reg(<pvName>, <providerName = "pva"/"ca">) command
template <typename T>
struct pvreg : public exprtk::igeneric_function<T>
{
public:

  typedef typename exprtk::igeneric_function<T> igfun_t;
  typedef typename igfun_t::parameter_list_t    parameter_list_t;
  typedef typename igfun_t::generic_type        generic_type;
  typedef typename generic_type::string_view    string_t;

  using exprtk::igeneric_function<T>::operator();

  pvreg()
  : exprtk::igeneric_function<T>("SS")
  { 
    printf("pvreg constructs 1\n"); 
  }

  inline T operator()(parameter_list_t parameters)
  {
    if (getEcmcEpicsIOCState()!=ECMC_IOC_STARTED_STATE) {    
      return -ECMC_PV_IOC_NOT_STARTED;
    }

    string_t pvName(parameters[0]);
    string_t providerName(parameters[1]);
    std::string pvNameStr(&pvName[0]);
    std::string providerNameStr(&providerName[0]);
    
    int index = -1;
    bool alreadyReg = false;
    try{
      //check if pv, provider combo already exist.. then erase and replace with new
      for(unsigned int i = 0; i < pvVector.size(); ++i) {
        if(pvVector.at(i)->getChannelName() == pvNameStr && 
           pvVector.at(i)->getProviderName() == providerNameStr) {
          ecmcPvPtr pvTemp = pvVector.at(i);
          pvVector.at(i) = NULL;
          index = i;
          alreadyReg = true;
          break;
        }
      }

      if(!alreadyReg) {
        // Pick first free
        for(unsigned int i = 0; i < pvVector.size(); ++i) {
          if(!(pvVector.at(i)->inUse())) {
            index = i;
            break;
          }
        }
      }

      PvaClientPtr pvaClient = PvaClient::get(providerNameStr);            
      // return handle to object (1 higher than index to avoid 0)      
      if(index>=0) {             // replace object
        //ecmcPvPtr pv = ecmcPv::create(pvaClient,pvNameStr,providerNameStr,"value",index+1);
        //pvVector.at(index) = pv; 
        pvVector.at(index)->regCmd(pvaClient,pvNameStr,providerNameStr,"value");
        //std::cerr << "Adding Pv : " << pvNameStr << " at index: " << index <<  "\n";
        return index + 1;        // Start count handles from 1
      } else {                   // Not found or no free objects to use..           
        std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_REG_ASYN  "(): failed for pv" << pvNameStr << "\n";
        //ecmcPvPtr pv = ecmcPv::create(pvaClient,pvNameStr,providerNameStr,"value",pvVector.size()+1);
        //pvVector.push_back(pv);
        return -ECMC_PV_REG_ERROR;
      }
    }    
    catch(std::exception &e){
      std::cerr << "Error: " ECMC_PV_PLC_CMD_PV_REG_ASYN  "(): " << e.what() << "\n";
      return T(-ECMC_PV_REG_ERROR);
    }
    
    return -ECMC_PV_REG_ERROR;
  }
};

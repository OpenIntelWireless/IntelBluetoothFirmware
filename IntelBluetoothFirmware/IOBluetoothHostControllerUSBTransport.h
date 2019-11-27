//
//  IOBluetoothHostControllerUSBTransport.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/11/19.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef IOBluetoothHostControllerUSBTransport_h
#define IOBluetoothHostControllerUSBTransport_h

#include "IOBluetoothHostControllerTransport.h"

class IOBluetoothHostControllerUSBTransport : public IOBluetoothHostControllerTransport
{
    OSDeclareAbstractStructors(IOBluetoothHostControllerUSBTransport)
    
public:

    virtual bool init(OSDictionary *){return true;};
    virtual void free(void){};
    virtual IOService * probe(IOService *,int *){return NULL;};
    virtual bool start(IOService *){return false;};
    virtual void stop(IOService *) {};
    virtual bool terminateWL(uint){return true;};
    virtual IOReturn ProcessG3StandByWake(void) {return 0;};
    virtual IOReturn InitializeTransportWL(IOService *){return 0;};
    virtual int MessageReceiver(void *,void *,uint,IOService *,void *,ulong){return 0;};
    virtual void AbortPipesAndClose(bool,bool){};
    virtual unsigned long maxCapabilityForDomainState(ulong){return 0;};
    virtual bool NeedToTurnOnUSBDebug(void){return 0;};
    virtual bool ConfigurePM(IOService *){return 0;};
    virtual unsigned long initialPowerStateForDomainState(ulong) {return 2;};
    virtual IOReturn RequestTransportPowerStateChange(IOBluetoothHCIControllerInternalPowerState,char *){return 0;};
    virtual void CompletePowerStateChange(char *){};
    virtual IOReturn ProcessPowerStateChangeAfterResumed(char *){return 0;};
    virtual IOReturn setPowerStateWL(ulong,IOService *){return 0;};
    virtual IOReturn powerStateWillChangeTo(ulong,ulong,IOService *){return 0;};
    virtual IOReturn powerStateWillChangeToWL(uint,void *){return 0;};
    virtual IOReturn SystemGoingToSleep(void){return 0;};
    virtual IOReturn PrepareControllerForSleep(void){return 0;};
    virtual IOReturn PrepareControllerWakeFromSleep(void){return 0;};
    virtual IOReturn PrepareControllerForPowerOff(bool){return 0;};
    virtual IOReturn PrepareControllerForPowerOn(void){return 0;};
    virtual bool SystemWakeCausedByBluetooth(void){return 0;};
    virtual IOReturn systemWillShutdownWL(uint,void *){return 0;};
    virtual bool ConfigureDevice(void){return 0;};
    virtual int GetInterfaceNumber(IOUSBHostInterface *){return 0;};
    virtual int FindNextInterface(IOUSBHostInterface *,ushort,ushort,ushort,ushort){return 0;};
    virtual int FindNextPipe(IOUSBHostInterface *,uchar,uchar,ushort *){return 0;};
    virtual int FindInterfaces(void){return 0;};
    virtual IOReturn StartInterruptPipeRead(void){return 0;};
    virtual IOReturn InterruptReadHandler(void *,void *,int,uint){return 0;};
    virtual bool StopInterruptPipeRead(void){return 0;};
    virtual IOReturn StartBulkPipeRead(void){return 0;};
    virtual IOReturn BulkInReadHandler(void *,void *,int,uint){return 0;};
    virtual bool StopBulkPipeRead(void){return 0;};
    virtual bool StartIsochPipeRead(void){return 0;};
    virtual IOReturn IsochInReadHandler(void *,void *,int,IOUSBHostIsochronousFrame *){return 0;};
    virtual bool StopIsochPipeRead(void){return 0;};
    virtual void ResetIsocFrames(IOUSBHostIsochronousFrame *,uint){};
    virtual IOReturn StopAllPipes(void){return 0;};
    virtual IOReturn StartAllPipes(void){return 0;};
    virtual IOReturn WaitForAllIOsToBeAborted(void){return 0;};
    virtual bool ReceiveInterruptData(void *,uint,bool){return 0;};
    virtual IOReturn TransportBulkOutWrite(void *) {return 1;};
    virtual IOReturn BulkOutWrite(IOMemoryDescriptor *){return 0;};
    virtual int BulkOutWriteTimerFired(OSObject *,IOTimerEventSource *){return 0;};
    virtual int BulkOutWriteCompleteHandler(void *,void *,int,uint){return 0;};
    virtual IOReturn BulkOutWriteCompleteAction(OSObject *,void *,void *,void *,void *,void *,void *){return 0;};
    virtual int HandleBulkOutWriteTimeout(IOBluetoothMemoryDescriptorRetainer *){return 0;};
    virtual int HandleIsochData(void *,int,IOUSBHostIsochronousFrame *){return 0;};
    virtual IOReturn TransportIsochOutWrite(void *,void *,int){return 0;};
    virtual int IsochOutWrite(IOMemoryDescriptor *,IOBluetoothSCOMemoryDescriptorRetainer *,int){return 0;};
    virtual int IsochOutWriteCompleteHandler(void *,void *,int,IOUSBHostIsochronousFrame *){return 0;};
    virtual IOReturn SendHCIRequest(uchar *,ulong long){return 0;};
    virtual IOReturn DeviceRequestCompleteHandler(void *,void *,int,uint){return 0;};
    virtual IOReturn DeviceRequestCompleteAction(OSObject *,void *,void *,void *,void *,void *,void *){return 0;};
    virtual IOReturn HandleMessageAction(OSObject *,void *,void *,void *,void *,void *,void *){return 0;};
    virtual IOReturn HandleMessage(uint,IOService *,void *,ulong){return 0;};
    virtual IOReturn DoDeviceReset(ushort){return 0;};
    virtual IOReturn HardReset(void){return 0;};
    virtual IOReturn UpdateSCOConnections(uchar,uint){return 0;};
    virtual void SetupTransportSCOParameters(void){};
    virtual void DestroyTransportSCOParameters(void){};
    virtual int LogData(void *,ulong long,ulong long){return 0;};
    virtual bool HostSupportsSleepOnUSB(void){return false;};
    virtual int USBControllerSupportsSuspend(void){return 0;};
    virtual IOReturn SetRemoteWakeUp(bool){return 0;};
    virtual bool StartLMPLogging(void){return 1;};
    virtual bool StartLMPLoggingBulkPipeRead(void){return 1;};
    virtual IOReturn ToggleLMPLogging(uchar *){return 0;};
    virtual IOReturn TransportLMPLoggingBulkOutWrite(uchar,uchar) {return 0;};
    virtual bool ControllerSupportWoBT(void){return 0;};
    virtual int GetControllerVendorID(void){return 0;};
    virtual int GetControllerProductID(void){return 0;};
    virtual int GetRadioPowerState(void) {return 0;};
    virtual void SetRadioPowerState(uchar){};
    virtual IOReturn ReConfigure(void){return 0;};
    virtual IOReturn ResetBluetoothDevice(void){return 0;};
    virtual int GetInfo(void *){return 0;};
    virtual int SetIdlePolicyValueAction(OSObject *,void *,void *,void *,void *) {return 0;};
    virtual IOReturn SetIdlePolicyValue(uint) {return 0;};
    virtual bool TransportWillReEnumerate(void){return false;};
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  0);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  1);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  2);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  3);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  4);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  5);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  6);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  7);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  8);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport,  9);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 10);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 11);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 12);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 13);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 14);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 15);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 16);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 17);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 18);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 19);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 20);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 21);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 22);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerUSBTransport, 23);
};

#endif /* IOBluetoothHostControllerUSBTransport_h */

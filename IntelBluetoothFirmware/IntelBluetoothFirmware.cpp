/* add your code here */
#include <IOKit/usb/USB.h>
#include <IOKit/IOCatalogue.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>
#include <sys/utfconv.h>
#include "BtIntel.h"

#include "IntelBluetoothFirmware.hpp"

#define super IOService
OSDefineMetaClassAndStructors(IntelBluetoothFirmware, IOService)

bool IntelBluetoothFirmware::init(OSDictionary *dictionary)
{
    IOLog("Driver init()\n");
    return super::init(dictionary);
}

void IntelBluetoothFirmware::free() {
    IOLog("Driver free()\n");
    super::free();
}

bool IntelBluetoothFirmware::start(IOService *provider)
{
    IOLog("Driver Start()\n");
    super::start(provider);
    return true;
}

void IntelBluetoothFirmware::stop(IOService *provider)
{
    IOLog("Driver Stop()\n");
    super::stop(provider);
}

IOService * IntelBluetoothFirmware::probe(IOService *provider, SInt32 *score)
{
    IOLog("Driver Probe()\n");
    if (!super::probe(provider, score)) {
        IOLog("super probe failed\n");
        return NULL;
    }
    m_pDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!m_pDevice) {
        IOLog("%s -- %s is not usb device\n", provider->getLocation(), provider->getName());
        return NULL;
    }
    m_pDevice->retain();
    UInt16 vendorID = USBToHost16(m_pDevice->getDeviceDescriptor()->idVendor);
    UInt16 productID = USBToHost16(m_pDevice->getDeviceDescriptor()->idProduct);
    IOLog("name=%s, vendorID=%02x, productID=%02x\n", m_pDevice->getName(), vendorID, productID);
    return this;
}

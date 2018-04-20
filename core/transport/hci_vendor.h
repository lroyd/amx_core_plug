#ifndef __HCI_VENDOR_H__
#define __HCI_VENDOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hci_lib.h"
#include "log_file.h"






#define SCHED_NORMAL 0


#ifndef HC_LINUX_BASE_POLICY
#define HC_LINUX_BASE_POLICY SCHED_NORMAL
#endif








extern hc_callbacks_t *hc_cbacks;




#ifdef __cplusplus
}
#endif
#endif




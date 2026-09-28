#pragma once

#undef USB_VID
#undef USB_PID
#undef USBD_VID
#undef USBD_PID
#define USB_VID  0x1209
#define USB_PID  0xC0BA
#define USBD_VID 0x1209
#define USBD_PID 0xC0BA

#undef USB_MANUFACTURER
#undef USB_MANUFACTURER_STRING
#undef USBD_MANUFACTURER_STRING
#define USB_MANUFACTURER         "Synarix"
#define USB_MANUFACTURER_STRING  "Synarix"
#define USBD_MANUFACTURER_STRING "Synarix"

#undef USB_PRODUCT
#undef USB_PRODUCT_STRING
#undef USBD_PRODUCT_STRING
#undef USBD_PRODUCT_FS_STRING
#undef USBD_PRODUCT_HS_STRING
#define USB_PRODUCT            "Cobalt-X Zero"
#define USB_PRODUCT_STRING     "Cobalt-X Zero"
#define USBD_PRODUCT_STRING    "Cobalt-X Zero"
#define USBD_PRODUCT_FS_STRING "Cobalt-X Zero"
#define USBD_PRODUCT_HS_STRING "Cobalt-X Zero"

#undef USBD_SIZ_STRING_SERIAL
#define USBD_SIZ_STRING_SERIAL 0x1A

#undef HID_FS_BINTERVAL
#define HID_FS_BINTERVAL 0x01

#define COBALT_MOUSE_DESC_SIZE 74
#define COBALT_MOUSE_DESC                                              \
  {                                                                    \
    0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 0x09, 0x01, 0xA1, 0x00,        \
    0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01,        \
    0x95, 0x03, 0x75, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x05,        \
    0x81, 0x01, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38,        \
    0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x03, 0x81, 0x06,        \
    0xC0, 0x09, 0x3C, 0x05, 0xFF, 0x09, 0x01, 0x15, 0x00, 0x25,        \
    0x01, 0x75, 0x01, 0x95, 0x02, 0xB1, 0x22, 0x75, 0x06, 0x95,        \
    0x01, 0xB1, 0x01, 0xC0                                             \
  }

#define COBALT_VENDOR_DESC_SIZE 35
#define COBALT_VENDOR_DESC                                             \
  {                                                                    \
    0x06, 0x00, 0xFF, 0x09, 0x00, 0xA1, 0x01, 0x85, 0x02, 0x15,        \
    0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x40, 0x09, 0x00,        \
    0x81, 0x02, 0x95, 0x40, 0x09, 0x00, 0x91, 0x02, 0x95, 0x40,        \
    0x09, 0x00, 0xB1, 0x02, 0xC0                                       \
  }

#define HID_MOUSE_REPORT_DESC_SIZE       COBALT_MOUSE_DESC_SIZE
#define HID_MOUSE_REPORT_DESC            COBALT_MOUSE_DESC
#define USBD_HID_MOUSE_REPORT_DESC_SIZE  COBALT_MOUSE_DESC_SIZE
#define USBD_HID_MOUSE_REPORT_DESC       COBALT_MOUSE_DESC

#define USBD_CUSTOM_HID_REPORT_DESC_SIZE COBALT_VENDOR_DESC_SIZE
#define USBD_CUSTOM_HID_REPORT_DESC      COBALT_VENDOR_DESC
#define USBD_CUSTOMHID_REPORT_DESC_SIZE  COBALT_VENDOR_DESC_SIZE
#define USBD_CUSTOMHID_REPORT_DESC       COBALT_VENDOR_DESC
#define CUSTOM_HID_REPORT_DESC_SIZE      COBALT_VENDOR_DESC_SIZE
#define CUSTOM_HID_REPORT_DESC           COBALT_VENDOR_DESC
#define CUSTOMHID_REPORT_DESC_SIZE       COBALT_VENDOR_DESC_SIZE
#define CUSTOMHID_REPORT_DESC            COBALT_VENDOR_DESC
#define HID_CUSTOM_HID_REPORT_DESC_SIZE  COBALT_VENDOR_DESC_SIZE
#define HID_CUSTOM_HID_REPORT_DESC       COBALT_VENDOR_DESC
#define HID_CUSTOMHID_REPORT_DESC_SIZE   COBALT_VENDOR_DESC_SIZE
#define HID_CUSTOMHID_REPORT_DESC        COBALT_VENDOR_DESC
#define CUSTOM_HID_REPORT_DESC_FS_SIZE   COBALT_VENDOR_DESC_SIZE
#define CUSTOM_HID_REPORT_DESC_FS        COBALT_VENDOR_DESC

#undef CUSTOM_HID_EPIN_SIZE
#undef CUSTOM_HID_EPOUT_SIZE
#undef CUSTOM_HID_FS_BINTERVAL
#undef USBD_CUSTOMHID_OUTREPORT_BUF_SIZE
#define CUSTOM_HID_EPIN_SIZE              0x40
#define CUSTOM_HID_EPOUT_SIZE             0x40
#define CUSTOM_HID_FS_BINTERVAL           0x01
#define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE 0x40

#include "usb.h"



#ifdef _USE_HW_USB
#include "cdc.h"
#include "cli.h"

static bool is_init = false;
static UsbMode_t is_usb_mode = USB_NON_MODE;


#if CLI_USE(HW_USB)
static void cliCmd(cli_args_t *args);
#endif




bool usbInit(void)
{
  tusb_rhport_init_t dev_init =
  {
    .role  = TUSB_ROLE_DEVICE,
    .speed = TUSB_SPEED_AUTO};

    
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }


  GPIO_InitTypeDef GPIO_InitStruct;
  
  // USB Pins TODO double check USB clock and pin setup
  // Configure USB DM and DP pins. This is optional, and maintained only for user guidance.
  GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral clock enable */
  __HAL_RCC_USB_CLK_ENABLE();

  /* Enable VDDUSB */
  #if defined (PWR_USBSCR_USB33DEN)
  HAL_PWREx_EnableVddUSB();
  #endif
  
  
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

#if CLI_USE(HW_USB)
  cliAdd("usb", cliCmd);
#endif
  return true;
}

void usbDeInit(void)
{
  if (is_init == true)
  {
  }
}

bool usbIsOpen(void)
{
  return cdcIsConnect();
}

bool usbIsConnect(void)
{
  bool ret = false;

  if (tud_connected() && !tud_suspended())
  {
    ret = true;
  }

  return ret;
}

UsbMode_t usbGetMode(void)
{
  return is_usb_mode;
}

UsbType_t usbGetType(void)
{
  return (UsbType_t)cdcGetType();
}

bool usbUpdate(void)
{
  tud_task();
  return true;
}

size_t usbGeUniqueID(uint8_t id[], size_t max_len)
{
  (void)max_len;
  volatile uint32_t *stm32_uuid = (volatile uint32_t *)UID_BASE;
  uint32_t          *id32       = (uint32_t *)(uintptr_t)id;
  uint8_t const      len        = 12;

  id32[0] = stm32_uuid[0];
  id32[1] = stm32_uuid[1];
  id32[2] = stm32_uuid[2];

  return len;
}

size_t usbGetSerial(uint16_t desc_str1[], size_t max_chars)
{
  uint8_t uid[16] TU_ATTR_ALIGNED(4);
  size_t  uid_len;


  uid_len = usbGeUniqueID(uid, sizeof(uid));

  if (uid_len > max_chars / 2) uid_len = max_chars / 2;

  for (size_t i = 0; i < uid_len; i++)
  {
    for (size_t j = 0; j < 2; j++)
    {
      const char nibble_to_hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
      uint8_t const nibble       = (uid[i] >> (j * 4)) & 0xf;
      desc_str1[i * 2 + (1 - j)] = nibble_to_hex[nibble]; // UTF-16-LE
    }
  }

  return 2 * uid_len;
}

void USB_DRD_FS_IRQHandler(void)
{
  tud_int_handler(BOARD_TUD_RHPORT);
}


#if CLI_USE(HW_USB)
void cliCmd(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    while(cliKeepLoop())
    {
      cliPrintf("mounted   : %d\n", tud_mounted());
      cliPrintf("connedted : %d\n", tud_connected());
      cliPrintf("tud_suspended : %d\n", tud_suspended());

      cliPrintf("USB Mode    : %d\n", usbGetMode());
      cliPrintf("USB Type    : %d\n", usbGetType());
      cliPrintf("USB Connect : %d\n", usbIsConnect());
      cliPrintf("USB Open    : %d\n", usbIsOpen());
      cliMoveUp(7);
      delay(100);
    }
    cliMoveDown(7);

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "tx") == true)
  {
    uint32_t pre_time;
    uint32_t tx_cnt = 0;
    uint32_t sent_len = 0;

    pre_time = millis();
    while(cliKeepLoop())
    {
      if (millis()-pre_time >= 1000)
      {
        pre_time = millis();
        logPrintf("tx : %d KB/s\n", tx_cnt/1024);
        tx_cnt = 0;
      }
      sent_len = cdcWrite((uint8_t *)"123456789012345678901234567890\n", 31);
      tx_cnt += sent_len;
      usbUpdate();
    }
    cliPrintf("\x1B[%dB", 2);

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "rx") == true)
  {
    uint32_t pre_time;
    uint32_t rx_cnt = 0;
    uint32_t rx_len;

    pre_time = millis();
    while(cliKeepLoop())
    {
      if (millis()-pre_time >= 1000)
      {
        pre_time = millis();
        logPrintf("rx : %d KB/s\n", rx_cnt/1024);
        rx_cnt = 0;
      }

      rx_len = cdcAvailable();

      for (int i=0; i<rx_len; i++)
      {
        cdcRead();
      }

      rx_cnt += rx_len;
    }
    cliPrintf("\x1B[%dB", 2);

    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("usb info\n");
    cliPrintf("usb tx\n");
    cliPrintf("usb rx\n");
  }
}
#endif

#endif
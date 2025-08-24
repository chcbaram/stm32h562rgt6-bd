#include "cdc.h"


#ifdef _USE_HW_CDC
#include "usb.h"



static bool is_init = false;

bool cdcInit(void)
{
  bool ret = true;

  is_init = ret;

  return ret;
}

bool cdcIsInit(void)
{
  return is_init;
}

bool cdcIsConnect(void)
{
  return tud_cdc_n_connected(0);
}

uint32_t cdcAvailable(void)
{
  return tud_cdc_n_available(0);
}

uint8_t cdcRead(void)
{
  uint8_t buf[1];

  tud_cdc_n_read(0, buf, 1);

  return buf[0];
}

uint32_t cdcWrite(uint8_t *p_data, uint32_t length)
{
  uint32_t pre_time;
  uint32_t tx_len;
  uint32_t sent_len;


  if (cdcIsConnect() != true) 
    return 0;


  sent_len = 0;

  pre_time = millis();
  while(sent_len < length)
  {
    usbUpdate();

    tx_len = length - sent_len;

    tx_len = tud_cdc_n_write(0, p_data, tx_len);

    if (tx_len > 0)
    {
      
      p_data += tx_len;
      sent_len += tx_len;
    }
    
    if (cdcIsConnect() != true)
    {
      break;
    }

    if (millis()-pre_time >= 100)
    {
      break;
    }
  }

  tud_cdc_n_write_flush(0);  

  return sent_len;  
}

uint32_t cdcGetBaud(void)
{
  // return cdcIfGetBaud();
  return 0;
}

uint8_t cdcGetType(void)
{
  // return cdcIfGetType();
  return 0;
}

void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts)
{
  (void)rts;

  // DTR = false is counted as disconnected
  if (!dtr)
  {
    // touch1200 only with first CDC instance (Serial)
    if (instance == 0)
    {
      cdc_line_coding_t coding;
      tud_cdc_get_line_coding(&coding);
      if (coding.bit_rate == 1200)
      {
      }
    }
  }
}
#endif

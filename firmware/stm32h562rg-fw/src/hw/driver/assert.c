#include "assert.h"



#ifdef USE_FULL_ASSERT
#include "cli.h"

#define ASSERT_MAX_LOG      5


#ifdef _USE_HW_CLI
static void cliAssert(cli_args_t *args);
#endif

typedef struct 
{
  char fail_file_name[ASSERT_MAX_LOG][50];
  uint32_t fail_file_line[ASSERT_MAX_LOG];
  uint32_t fail_cnt;
} fail_log_file_t;

static fail_log_file_t fail_file;

static bool is_init = false;






bool assertInit(void)
{
  is_init = true;

#ifdef _USE_HW_CLI
  cliAdd("assert", cliAssert);
#endif  
  return true;
}

void assert_failed(uint8_t* file, uint32_t line)
{
  char *file_name_buf;
  

  if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)  
  { 
    #if defined(__GNUC__)
    __asm__ __volatile__ ("bkpt #0"); 
    #elif defined(__ICCARM__)
    __asm("bkpt 0x00");
    #endif
  }

  if (strrchr((char *) file,'/') == NULL) 
  {
    file_name_buf = strrchr((char *)file,'\\')+1;
  }
  else 
  {
    file_name_buf = strrchr((char *)file,'/')+1;
  }

  if (fail_file.fail_cnt < ASSERT_MAX_LOG)
  {
    strcpy(fail_file.fail_file_name[fail_file.fail_cnt], file_name_buf);
    fail_file.fail_file_line[fail_file.fail_cnt] = line;
    fail_file.fail_cnt++;

    if (is_init)
    {
      logPrintf("assert_failed() file: %s  line :%d\n", file_name_buf, line);
    }
  }
}

#ifdef _USE_HW_CLI
void cliAssert(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("assert cnt : %d\n", fail_file.fail_cnt);
    if (fail_file.fail_cnt > 0)
    {      
      for (int i=0; i < (int)fail_file.fail_cnt; i++)
      {
        cliPrintf("file: %s  line :%d\n", fail_file.fail_file_name[i], fail_file.fail_file_line[i]);
      }
    }    
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "clear"))
  {
    fail_file.fail_cnt = 0;
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("assert info\n");
    cliPrintf("assert clear\n");
  }
}
#endif

#endif

#ifndef ASSERT_H_
#define ASSERT_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_ASSERT

#ifndef USE_FULL_ASSERT
#error "Must Define USE_FULL_ASSERT"
#endif

#include "stm32_assert.h"



bool assertInit(void);



#endif

#ifdef __cplusplus
 }
#endif


#endif 
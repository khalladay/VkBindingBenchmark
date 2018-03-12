#pragma once

#define UBO_TEST 0
#define SSBO_TEST 1
#define PUSH_TEST 0

#define DEVICE_LOCAL 0 

#if UBO_TEST
#define data_store ubo_store
#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\ubo_array.vert.spv"
#include "ubo_store.h"
#elif SSBO_TEST
#define data_store ssbo_store
#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\ssbo_array.vert.spv"
#include "ssbo_store.h"
#else
#define data_store null_store
#define VERT_SHADER_NAME ""
#include "null_store.h"
#endif

static_assert( ((UBO_TEST  + SSBO_TEST == 0)  || (UBO_TEST  != SSBO_TEST)), "Cannot have more than one test active");
static_assert( ((UBO_TEST  + PUSH_TEST == 0)  || (UBO_TEST  != PUSH_TEST)), "Cannot have more than one test active");
static_assert( ((SSBO_TEST + PUSH_TEST == 0)  || (SSBO_TEST != PUSH_TEST)), "Cannot have more than one test active");

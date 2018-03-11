#pragma once

#define UBO_TEST 1
#define SSBO_TEST 0
#define PUSH_TEST 0

#if UBO_TEST
#define data_store ubo_store
#include "ubo_store.h"
#elif SSBO_TEST
#define data_store ssbo_store
#include "ssbo_store.h"
#else
#define data_store null_store
#include "null_store.h"
#endif

static_assert( ((UBO_TEST  + SSBO_TEST == 0)  || (UBO_TEST  != SSBO_TEST)), "Cannot have more than one test active");
static_assert( ((UBO_TEST  + PUSH_TEST == 0)  || (UBO_TEST  != PUSH_TEST)), "Cannot have more than one test active");
static_assert( ((SSBO_TEST + PUSH_TEST == 0)  || (SSBO_TEST != PUSH_TEST)), "Cannot have more than one test active");

#define GLOBAL_TEXTURE_ARRAY 1
#define GLOBAL_UBO_ARRAY 1
#define PER_OBJECT_UBO 0
#define GLOBAL_SSBO_ARRAY 0

#pragma once

#define SCREEN_W 1280
#define SCREEN_H 720

//Test Categories
#define UBO_TEST 0
#define SSBO_TEST 1
#define PUSH_TEST 0

//What mesh to test
#define BISTRO_TEST 0
#define SPONZA_TEST 1

//Test Modifiers
#define DYNAMIC_UBO 0
#define DEVICE_LOCAL 1
#define WITH_VK_TIMESTAMP 0
#define PERSISTENT_STAGING_BUFFER 1
#define COPY_ON_MAIN_COMMANDBUFFER 1
#define COMBINE_MESHES 0
#define SHUFFLE_MESHES 1

#if UBO_TEST
	#define data_store ubo_store
	#if DYNAMIC_UBO	
		#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\dynamic_ubo.vert.spv"
	#else
		#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\ubo_array.vert.spv"
	#endif
	#include "ubo_store.h"
#elif SSBO_TEST
	#define data_store ssbo_store
	#if BISTRO_TEST
		#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\ssbo_array.vert.spv"
	#else
		#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\ssbo_array_511.vert.spv"
	#endif
	#include "ssbo_store.h"
#else
	#define data_store null_store
	#define VERT_SHADER_NAME ""
	#include "null_store.h"
#endif

static_assert( ((UBO_TEST  + SSBO_TEST == 0)    || (UBO_TEST  != SSBO_TEST)), "Cannot have more than one test active");
static_assert( ((UBO_TEST  + PUSH_TEST == 0)    || (UBO_TEST  != PUSH_TEST)), "Cannot have more than one test active");
static_assert( ((SSBO_TEST + PUSH_TEST == 0)    || (SSBO_TEST != PUSH_TEST)), "Cannot have more than one test active");
static_assert(  PERSISTENT_STAGING_BUFFER == 0	|| PERSISTENT_STAGING_BUFFER == DEVICE_LOCAL, "PERSISTENT_STAGING_BUFFER requires DEVICE_LOCAL to function");
static_assert(  COPY_ON_MAIN_COMMANDBUFFER == 0 || COPY_ON_MAIN_COMMANDBUFFER == DEVICE_LOCAL, "COPY_ON_MAIN_COMMANDBUFFER requires DEVICE_LOCAL to function");
static_assert(COPY_ON_MAIN_COMMANDBUFFER + PERSISTENT_STAGING_BUFFER == 0 || !COPY_ON_MAIN_COMMANDBUFFER || COPY_ON_MAIN_COMMANDBUFFER + PERSISTENT_STAGING_BUFFER == 2, "Must have UBO_TEST defined to use DYNAMIC_UBO");
static_assert(UBO_TEST + DYNAMIC_UBO == 0 || !DYNAMIC_UBO || UBO_TEST + DYNAMIC_UBO == 2, "Must have UBO_TEST defined to use DYNAMIC_UBO");

//Results
/*
Sponza
			| (No Modifier) | DYNAMIC_UBO | DEVICE_LOCAL | COMBINE_MESHES |  SHUFFLE_MESHES	|
UBO_TEST	|		1.01	|	1.09	  |		0.97	 |		0.73	  |		1.05		|
SSBO_TEST	|		3.08	|			  |		0.89	 |		1.56	  |		2.53		|
PUSH_TEST   |		0.74    |			  |				 |		0.70	  |		0.73		|

Interior+Exterior
			| (No Modifier) 	| DYNAMIC_UBO | DEVICE_LOCAL | COMBINE_MESHES | SHUFFLE_MESHES	|  
UBO_TEST	|		5.9			|	5.9		  |		15.41	 |		2.07	  |			6.46	|
SSBO_TEST	|		42.23		|			  |		8.38	 |		7.82	  |			41.92	|
PUSH_TEST   |		9.37		|			  |				 |		1.66	  |			10.30	|

*/

/*
TESTING METHOD - set to "cool" mode, set to "high performance" , plugged in

Using Open hardware monitor, wait for CPU cores to be below 65 C

Numbers are an average of 4096 frames, not using the first 1024 frames of the application

SHUFFLE_MESHES is ALWAYS ON 
COMBINE MESHES IS ALWAYS OFF

Sponza -> SSBO set to 511 -> 394 meshes
			| (No Modifier) | DYNAMIC_UBO | DEVICE_LOCAL | DEVICE_LOCAL + DYNAMIC UBO
UBO_TEST	|		0.83	|		0.78  |		1.61	 |  0.96
SSBO_TEST	|		2.57	|			  |		0.89	 |
PUSH_TEST   |		0.77    |			  |				 |
-------------
			| Persistent staging    |   BOTH 
UBO_TEST	|		0.95			|	0.79	
DYNAMIC_UBO	|		0.95			|	0.80
SSBO_TEST	|		0.89			|	0.79
------------



Bistro -> SSBO set to 25000 -> 24777 meshes
			| (No Modifier) | DYNAMIC_UBO | DEVICE_LOCAL | DEVICE_LOCAL + DYNAMIC UBO
UBO_TEST	|		6.57	|	6.93	  |		16.44	 |  17.36
SSBO_TEST	|		41.42	|			  |		7.84	 |
PUSH_TEST   |		10.3    |			  |				 |
-------------
			| Persistent staging    |	BOTH (persistent staging + main command buffer)
UBO_TEST	|		16.0			|	6.63
DYNAMIC_UBO	|		16.57			|	7.20
SSBO_TEST	|		7.43			|	6.69
------------

Notes: UBO_STORE set to 256 per page, when non dynamic, saw no real gains switching this to 511 (was the same as the BOTH test with 256)

Note: Persistent staging buffer without being on the main buffer is faster if it isn't host visible, on the mian buffer, it's faster host visible, wtf? 
*/
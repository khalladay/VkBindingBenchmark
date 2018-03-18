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
#define PERSISTENT_STAGING_BUFFER 0
#define COPY_ON_MAIN_COMMANDBUFFER 0
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
	#define VERT_SHADER_NAME "..\\data\\_generated\\builtshaders\\ssbo_array.vert.spv"
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

static_assert(UBO_TEST + DYNAMIC_UBO == 0 || !DYNAMIC_UBO || UBO_TEST + DYNAMIC_UBO == 2, "Must have UBO_TEST defined to use DYNAMIC_UBO");

//Results
/*
Sponza
			| (No Modifier) | DYNAMIC_UBO | DEVICE_LOCAL | COMBINE_MESHES |  SHUFFLE_MESHES	|
UBO_TEST	|		1.01	|	1.09	  |		1.62	 |		0.73	  |		1.05		|
SSBO_TEST	|		4.55	|			  |		5.73	 |		4.55	  |		4.55		|
PUSH_TEST   |		0.74    |			  |				 |		0.70	  |		0.73		|

Interior+Exterior
			| (No Modifier) / Timestamp	| DYNAMIC_UBO | DEVICE_LOCAL | COMBINE_MESHES | SHUFFLE_MESHES	|  
UBO_TEST	|		5.84 / 3.84			|	5.9		  |		15.41	 |		2.07	  |			6.46	|
SSBO_TEST	|		42.23				|			  |		8.38	 |		7.82	  |			41.92	|
PUSH_TEST   |		9.37				|			  |				 |		1.66	  |			10.30	|


// COPY ON MAIN COMMAND BUFFER -> ALL DEVICE_LOCAL + SHUFFLE_MESHES
Sponza
			|  PER FRAME STAGING | PERSISTENT STAGING | DYNAMIC_UBO | 
UBO_TEST	|		1.07		 |		1.03		  |				| 
SSBO_TEST	|					 |					  |				|
PUSH_TEST   |					 |					  |				|	


Interior+Exterior
			| (No Modifier) / Timestamp	| DYNAMIC_UBO | DEVICE_LOCAL | COMBINE_MESHES | SHUFFLE_MESHES	|
UBO_TEST	|		5.84 / 3.84			|	5.9		  |		15.41	 |		2.07	  |			6.46	|
SSBO_TEST	|		42.23				|			  |		8.38	 |		7.82	  |			41.92	|
PUSH_TEST   |		9.37				|			  |				 |		1.66	  |			10.30	|




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
SSBO_TEST	|		2.57	|			  |				 |
PUSH_TEST   |			    |			  |				 |

*/
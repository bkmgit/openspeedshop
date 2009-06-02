/*******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
** Copyright (c) 2007 William Hachfeld. All Rights Reserved.
** Copyright (c) 2007 Krell Institute.  All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/

/** @file
 *
 * Declaration and definition of the HWC sampling collector's runtime.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "RuntimeAPI.h"
#include "blobs.h"
#include "PapiAPI.h"

/** Type defining the items stored in thread-local storage. */
typedef struct {

    OpenSS_DataHeader header;  /**< Header for following data blob. */
    hwc_data data;             /**< Actual data blob. */

    OpenSS_PCData buffer;      /**< PC sampling data buffer. */

    int EventSet;
} TLS;

static int hwc_papi_init_done = 0;

#ifdef USE_EXPLICIT_TLS

/**
 * Thread-local storage key.
 *
 * Key used for looking up our thread-local storage. This key <em>must</em>
 * be globally unique across the entire Open|SpeedShop code base.
 */
static const uint32_t TLSKey = 0x00001EF5;

#else

/** Thread-local storage. */
static __thread TLS the_tls;

#endif


#if defined (OPENSS_OFFLINE)

extern void offline_sent_data(int);

void hwc_resume_papi()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = OpenSS_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    if (hwc_papi_init_done == 0 || tls == NULL)
        return;
    OpenSS_Start(tls->EventSet);
}

void hwc_suspend_papi()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = OpenSS_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    if (hwc_papi_init_done == 0 || tls == NULL)
        return;
    OpenSS_Stop(tls->EventSet);
}
#endif


/**
 * PAPI event handler.
 *
 * Called by PAPI each time a sample is to be taken. Takes the program counter
 * (PC) address passed by PAPI and places it into the sample buffer. When the
 * sample buffer is full, it is sent to the framework for storage in the
 * experiment's database.
 *
 * @note    
 * 
 * @param context    Thread context at papi overflow.
 */
static void hwcPAPIHandler(int EventSet, void* pc, 
			   long_long overflow_vector, void* context)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = OpenSS_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* Update the sampling buffer and check if it has been filled */
    if(OpenSS_UpdatePCData((uint64_t)pc, &tls->buffer)) {

	/* Send these samples */
	tls->header.time_end = OpenSS_GetTime();
	tls->header.addr_begin = tls->buffer.addr_begin;
	tls->header.addr_end = tls->buffer.addr_end;
	tls->data.pc.pc_len = tls->buffer.length;
	tls->data.count.count_len = tls->buffer.length;


#ifndef NDEBUG
        if (getenv("OPENSS_DEBUG_COLLECTOR") != NULL) {
            fprintf(stderr,"hwcPAPIHandler sends data:\n");
            fprintf(stderr,"time_end(%#lu) addr range [%#lx, %#lx] pc_len(%d) count_len(%d)\n",
                tls->header.time_end,tls->header.addr_begin,
		tls->header.addr_end,tls->data.pc.pc_len,
                tls->data.count.count_len);
        }
#endif

	OpenSS_SetSendToFile(&(tls->header), "hwc", "openss-data");
	OpenSS_Send(&tls->header, (xdrproc_t)xdr_hwc_data, &tls->data);

#if defined(OPENSS_OFFLINE)
        offline_sent_data(1);
#endif

	/* Re-initialize the data blob's header */
	tls->header.time_begin = tls->header.time_end;

	/* Re-initialize the sampling buffer */
	tls->buffer.addr_begin = ~0;
	tls->buffer.addr_end = 0;
	tls->buffer.length = 0;
	memset(tls->buffer.hash_table, 0, sizeof(tls->buffer.hash_table));

    }
}



/**
 * Start sampling.
 *
 * Starts hardware counter (HWC) sampling for the thread executing this
 * function. Initializes the appropriate thread-local data structures and
 * then enables the sampling counter.
 *
 * @param arguments    Encoded function arguments.
 */
void hwc_start_sampling(const char* arguments)
{
    hwc_start_sampling_args args;

    /* Create and access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = malloc(sizeof(TLS));
    Assert(tls != NULL);
    OpenSS_SetTLS(TLSKey, tls);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* Decode the passed function arguments */
    memset(&args, 0, sizeof(args));
    OpenSS_DecodeParameters(arguments,
			    (xdrproc_t)xdr_hwc_start_sampling_args,
			    &args);

    int hwc_papithreshold = (uint64_t)(args.sampling_rate);
    
    /* 
     * Initialize the data blob's header
     *
     * Passing &tls->header to OpenSS_InitializeDataHeader() was found
     * to not be safe on IA64 systems. Hopefully the extra copy can be
     * removed eventually.
     */
    
    OpenSS_DataHeader local_data_header;
    OpenSS_InitializeDataHeader(args.experiment, args.collector,
				&local_data_header);
    memcpy(&tls->header, &local_data_header, sizeof(OpenSS_DataHeader));
    
    /* Initialize the actual data blob */
    tls->data.interval = (uint64_t)(args.sampling_rate);
    tls->data.pc.pc_val = tls->buffer.pc;
    tls->data.count.count_val = tls->buffer.count;

    /* Initialize the sampling buffer */
    tls->buffer.addr_begin = ~0;
    tls->buffer.addr_end = 0;
    tls->buffer.length = 0;
    memset(tls->buffer.hash_table, 0, sizeof(tls->buffer.hash_table));

    /* Begin sampling */
    tls->header.time_begin = OpenSS_GetTime();

    if(hwc_papi_init_done == 0) {
	hwc_init_papi();
	tls->EventSet = PAPI_NULL;
	hwc_papi_init_done = 1;
    }

    int hwc_papi_event = PAPI_NULL;
#if defined (OPENSS_OFFLINE)
    const char* hwc_event_param = getenv("OPENSS_HWC_EVENT");
    if (hwc_event_param != NULL) {
        hwc_papi_event=get_papi_eventcode((char *)hwc_event_param);
    } else {
        hwc_papi_event=get_papi_eventcode("PAPI_TOT_CYC");
    }
#else
    hwc_papi_event = args.hwc_event;
#endif

    OpenSS_Create_Eventset(&tls->EventSet);
    OpenSS_AddEvent(tls->EventSet, hwc_papi_event);
    OpenSS_Overflow(tls->EventSet, hwc_papi_event,
		    hwc_papithreshold, hwcPAPIHandler);
    OpenSS_Start(tls->EventSet);
}



/**
 * Stop sampling.
 *
 * Stops hardware counter (HWC) sampling for the thread executing this function.
 * Disables the sampling counter and sends any samples remaining in the buffer.
 *
 * @param arguments    Encoded (unused) function arguments.
 */
void hwc_stop_sampling(const char* arguments)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = OpenSS_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* Stop sampling */
    OpenSS_Stop(tls->EventSet);
    tls->header.time_end = OpenSS_GetTime();

    /* Are there any unsent samples? */
    if(tls->buffer.length > 0) {

	/* Send these samples */
	tls->header.addr_begin = tls->buffer.addr_begin;
	tls->header.addr_end = tls->buffer.addr_end;
	tls->data.pc.pc_len = tls->buffer.length;
	tls->data.count.count_len = tls->buffer.length;

#ifndef NDEBUG
	if (getenv("OPENSS_DEBUG_COLLECTOR") != NULL) {
	    fprintf(stderr, "hwc_stop_sampling:\n");
	    fprintf(stderr, "time_end(%#lu) addr range[%#lx, %#lx] pc_len(%d) count_len(%d)\n",
		tls->header.time_end,tls->header.addr_begin,
		tls->header.addr_end,tls->data.pc.pc_len,
		tls->data.count.count_len);
	}
#endif

	OpenSS_SetSendToFile(&(tls->header), "hwc", "openss-data");
	OpenSS_Send(&(tls->header), (xdrproc_t)xdr_hwc_data, &(tls->data));

#if defined(OPENSS_OFFLINE)
        offline_sent_data(1);
#endif
	
    }

    /* Destroy our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    free(tls);
    OpenSS_SetTLS(TLSKey, NULL);
#endif
}

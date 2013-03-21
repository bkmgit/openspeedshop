////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 The Krell Institute. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file
 *
 * Declaration and definition of the MemDetail structure.
 *
 */

#ifndef _MemDetail_
#define _MemDetail_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TotallyOrdered.hxx"


namespace OpenSpeedShop { namespace Framework {

    /**
     * Memory event tracing collector details. 
     *
     * Encapsulate the details metric (inclusive or exclusive) for the Mem event
     * tracing collector.
     */
    struct MemDetail :
	public TotallyOrdered<MemDetail>
    {
	TimeInterval dm_interval;  /**< Begin/End time of the call. */
	double dm_time;            /**< Time spent in the call. */	
	uint64_t dm_retval;       /**< return value. */
	uint64_t dm_ptr;          /**< ptr arg */
	uint64_t dm_size1;        /**< size 1 arg*/
	uint64_t dm_size2;        /**< size 2 arg*/
	int dm_memtype;   /**< enumerated val which mem call is it */

	/** Operator "<" defined for two MemDetail objects. */
	bool operator<(const MemDetail& other) const
	{
	    if(dm_interval < other.dm_interval)
                return true;
            if(dm_interval > other.dm_interval)
                return false;
// FIXME: Need to complete the compare rules here.
            if(dm_retval < other.dm_retval)
                return true;
	    return dm_time < other.dm_time;
        }

    };
    
} }



#endif
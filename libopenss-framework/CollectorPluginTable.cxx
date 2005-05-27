////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
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
 * Definition of the CollectorPluginTable class.
 *
 */

#include "CollectorImpl.hxx"
#include "CollectorPluginTable.hxx"
#include "Guard.hxx"

#include <ltdl.h>
#include <stdlib.h>

using namespace OpenSpeedShop::Framework;



/** Singleton collector plugin table. */
CollectorPluginTable CollectorPluginTable::TheTable;



namespace {



    /**
     * Collector plugin search callback.
     *
     * Simply re-routes callbacks made during a collector plugin search to the
     * appropriate CollectorPluginTable object.
     *
     * @param filename    File name of potential collector plugin.
     * @param data        Untyped pointer to the CollectorPluginTable.
     * @return            Always returns zero to insure search continues.
     */
    int foreachCallback(const char* filename, lt_ptr data)
    {
	// Re-route the callback to the actual PluginTable object
	CollectorPluginTable* table =
	    reinterpret_cast<CollectorPluginTable*>(data);
	table->foreachCallback(filename);
	
	// Always return zero to insure we keep searching
	return 0;
    } 



}



/**
 * Default constructor.
 *
 * Sets up an empty collector plugin table and initializes libltdl. The actual
 * table isn't actually built until it is first required.
 */
CollectorPluginTable::CollectorPluginTable() :
    Lockable(),
    dm_is_built(false),
    dm_unique_id_to_entry()
{
    // Initialize libltdl
    Assert(lt_dlinit() == 0);
}



/**
 * Destructor.
 *
 * Destroys the collector plugin table and exits libltdl.
 *
 * @pre    All collector implementation instances associated with plugins in
 *         this table must be destroyed prior to the destruction of the table
 *         itself. An assertion failure occurs if any such instances remain.
 */
CollectorPluginTable::~CollectorPluginTable()
{
    // Check preconditions    
    for(std::map<std::string, Entry>::const_iterator
	    i = dm_unique_id_to_entry.begin();
	i != dm_unique_id_to_entry.end();
	++i) {
	Assert(i->second.instances == 0);
	Assert(i->second.handle == NULL);
    }
    
    // Exit libltdl
    Assert(lt_dlexit() == 0);
}



/**
 * Get all available collectors implementations.
 *
 * Returns the metadata for all available collector implementations. An empty
 * set is returned if no collector plugins were found.
 *
 * @return    Metadata for all available collector implementations.
 */
std::set<Metadata> CollectorPluginTable::getAvailable()
{
    std::set<Metadata> metadata;

    // Build the collector plugin table if necessary
    if(!dm_is_built)
	build();
    Assert(dm_is_built);
    
    // Assemble the metadata for all available collector implementations
    for(std::map<std::string, Entry>::const_iterator
	    i = dm_unique_id_to_entry.begin();
	i != dm_unique_id_to_entry.end();
	++i)
	metadata.insert(i->second.metadata);
    
    // Return the metadata to the caller
    return metadata;
}



/**
 * Instantiate a collector implementation.
 *
 * Instantiates a collector implementation by calling the corresponding plugin's
 * factory method and incrementing the corresponding plugin's instance count. If
 * the plugin's instance count was zero, the plugin is automatically loaded into
 * memory first. A null value is returned if the collector implementation cannot
 * be instantiated for any reason (plugin wasn't found, plugin was moved, plugin
 * was changed, etc.)
 *
 * @param unique_id    Unique identifier of the collector implementation to be
 *                     instantiated.
 * @return             New instance of this collector implementation or null if
 *                     the instantiation failed for any reason.
 */
CollectorImpl* CollectorPluginTable::instantiate(const std::string& unique_id)
{
    CollectorImpl* instance = NULL;

    // Build the collector plugin table if necessary
    if(!dm_is_built)
	build();
    Assert(dm_is_built);
    
    // Find the entry for this unique identifier's plugin
    std::map<std::string, Entry>::iterator
	i = dm_unique_id_to_entry.find(unique_id);
    if(i == dm_unique_id_to_entry.end())
	return NULL;
    
    // Critical section touching the collector plugin's entry
    {
	Guard guard_myself(this);
		
	// Is this the first instance of this collector implementation?
	if(i->second.instances == 0) {
	    
	    // Load the plugin into memory
	    i->second.handle = lt_dlopenext(i->second.path.c_str());
	    if(i->second.handle == NULL)
		return NULL;
	    
	}
		
	// Find the factory method in the plugin
	lt_dlhandle handle = reinterpret_cast<lt_dlhandle>(i->second.handle);
	Assert(handle != NULL);
	CollectorImpl* (*factory)() = (CollectorImpl* (*)())
	    lt_dlsym(handle, "CollectorFactory");
	if(factory == NULL)
	    return NULL;
	
	// Create an instance of this collector
	instance = (*factory)();
	if(i->second.metadata != *instance)
	    return NULL;
		
	// Increment the plugin's instance count
	i->second.instances++;    
	
    }
    
    // Return the instance to the caller
    return instance;
}

	
	
/**
 * Destroy a collector implementation instance.
 *
 * Destroys a collector implementation instance by destorying the actual
 * CollectorImpl object and decrementing the corresponding plugin's instance
 * count. If the plugin's instance count has reached zero, the plugin is
 * automatically unloaded from memory.
 *
 * @param impl    Collector implementation instance to be destroyed.
 */
void CollectorPluginTable::destroy(CollectorImpl* impl)
{
    // Check assertions
    Assert(dm_is_built);
    Assert(impl != NULL);
    
    // Find the entry for this collector's plugin
    std::map<std::string, Entry>::iterator
	i = dm_unique_id_to_entry.find(impl->getUniqueId());
    Assert(i != dm_unique_id_to_entry.end());
    
    // Destroy the implementation
    delete impl;
    
    // Critical section touching the collector plugin's entry
    {
	Guard guard_myself(this);
	
	// Decrement the plugin's instance count
	i->second.instances--;
	
	// Was this the last instance of this plugin?
	if(i->second.instances == 0) {
	    
	    // Unload the plugin from memory
	    lt_dlhandle handle =
		reinterpret_cast<lt_dlhandle>(i->second.handle);
	    Assert(i->second.handle != NULL);
	    Assert(lt_dlclose(handle) == 0);
	    i->second.handle = NULL;
	    
	}
    }
}


	
/**
 * Collector plugin search callback.
 *
 * Callback function called (indirectly via ::foreachCallback) by libltdl for
 * each potential collector plugin candidate found in our specified search path.
 * The candidate is examined to determine if it is truly a collector plugin and,
 * if so, is added to our table of available collector plugins.
 *
 * @note    The collector implementation instances created by each plugin must
 *          have a unique identifier different from that of any other plugin.
 *          Once a given unique identifier has been associated with a given
 *          plugin, any attempt by a subsequent plugin to use that unique 
 *          identifier will result in it being silently ignored.
 *
 * @param filename    File name of potential collector plugin.
 */
void CollectorPluginTable::foreachCallback(const std::string& filename)
{
    // Create an entry for this possible collector plugin
    Entry entry;
    entry.path = filename;
	    
    // Can we open this file as a libltdl module?
    lt_dlhandle handle = lt_dlopenext(entry.path.c_str());
    if(handle == NULL)
	return;
    
    // Is there a collector factory method in this module?
    CollectorImpl* (*factory)() = (CollectorImpl* (*)())
	lt_dlsym(handle, "CollectorFactory");
    if(factory == NULL) {
	Assert(lt_dlclose(handle) == 0);
	return;
    }
    
    // Get the metadata for this collector implementation
    CollectorImpl* instance = (*factory)();
    entry.metadata = *instance;
    delete instance;
	    
    // Close the module handle
    Assert(lt_dlclose(handle) == 0);
	    
    // Have we already used this unique identifier?
    if(dm_unique_id_to_entry.find(entry.metadata.getUniqueId()) != 
       dm_unique_id_to_entry.end())
	return;
	    
    // Add this entry to the table of collector plugins
    dm_unique_id_to_entry.insert(
	std::make_pair(entry.metadata.getUniqueId(), entry)
	);
}



/**
 * Build the table.
 *
 * Builds the table of available collector plugins. If libltdl's user-defined
 * library search path has not been set elsewhere (e.g. by the tool), it is set
 * to the value of the environment variable OPENSS_PLUGIN_PATH. In either case,
 * that path is then searched for plugins.
 */
void CollectorPluginTable::build()
{
    // Check preconditions
    Assert(!dm_is_built);
    
    // Only set the libltdl user-defined search path if it isn't already
    if((lt_dlgetsearchpath() == NULL) && (getenv("OPENSS_PLUGIN_PATH") != NULL))
	Assert(lt_dlsetsearchpath(getenv("OPENSS_PLUGIN_PATH")) == 0);
    
    // Search for collector plugins in the libltdl user-defined search path
    if(lt_dlgetsearchpath() != NULL)
	lt_dlforeachfile(lt_dlgetsearchpath(), ::foreachCallback, this);
    
    // Indicate this table has now been built.
    dm_is_built = true;
}

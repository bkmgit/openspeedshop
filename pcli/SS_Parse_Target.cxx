/** @file
 *
 * Storage for the parse results of a single given host/file/cluster
 * rank/pid/thread Target.
 *
 */

#include <vector>
#include <iostream>

using namespace std;

#include "SS_Parse_Range.hxx"
#include "SS_Parse_Target.hxx"
#include "SS_Parse_Result.hxx"

using namespace OpenSpeedShop::cli;

/**
 * Method: ParseTarget::XXX()
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
//void
//ParseTarget::
//XXX()
//{
//    return ;
//}
 
/**
 * Method: ParseTarget::pushHostPoint(char * name)
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
void
ParseTarget::
pushHostPoint(char * name)
{
    this->dm_used = true;   // All insertions need to do this currently
    ParseRange range(name);
    dm_host_list.push_back(range);
    return ;
}
 
/**
 * Method: ParseTarget::pushHostPoint(int num)
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
void
ParseTarget::
pushHostPoint(int num)
{
    this->dm_used = true;   // All insertions need to do this currently
    ParseRange range(num);
    dm_host_list.push_back(range);
    return ;
}
 
/**
 * Method: ParseTarget::pushHostRange(char * name, int num)
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
void
ParseTarget::
pushHostRange(char * name, int num)
{
    this->dm_used = true;   // All insertions need to do this currently
    ParseRange range(name,num);
    dm_host_list.push_back(range);
    return ;
}
 
/**
 * Method: ParseTarget::pushHostRange(char * name1, char * name2)
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
void
ParseTarget::
pushHostRange(char * name1, char * name2)
{
    this->dm_used = true;   // All insertions need to do this currently
    ParseRange range(name1,name2);
    dm_host_list.push_back(range);
    return ;
}
 
/**
 * Method: ParseTarget::pushHostRange(int num, char * name)
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
void
ParseTarget::
pushHostRange(int num, char * name)
{
    this->dm_used = true;   // All insertions need to do this currently
    ParseRange range(num,name);
    dm_host_list.push_back(range);
    return ;
}
 
/**
 * Method: ParseTarget::pushHostRange( int num1, int num2)
 * 
 *     
 * @return  .
 *
 * @todo    Error handling.
 *
 */
void
ParseTarget::
pushHostRange( int num1, int num2)
{
    this->dm_used = true;   // All insertions need to do this currently
    ParseRange range(num1,num2);
    dm_host_list.push_back(range);
    return ;
}
 

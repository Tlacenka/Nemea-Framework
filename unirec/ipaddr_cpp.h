/**
 * \file ipaddr_cpp.h
 * \brief Class to store both IPv4 and IPv6 addresses and associated methods. Uses ipaddr.h
 * \author Lukas Hutak <xhutak01@stud.fit.vutbr.cz>
 * \author Katerina Pilatova <xpilat05@stud.fit.vutbr.cz>
 * \date 2013
 * \date 2016
 */

/*
 * Copyright (C) 2013,2014,2015,2016 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#ifndef _IPADDR_CPP_H
#define _IPADDR_CPP_H

#include "ipaddr.h"
#include <string>
#include <iostream>
#include <arpa/inet.h>     // INET6_ADDRSTRLEN value

class IPaddr_cpp {
   bool new_object;
   const ip_addr_t *data;
public:

   /**
    * \brief Class constructor. Creates new ip_addr_t object.
    */
   inline IPaddr_cpp();

   /**
    * \brief Alternative constructor.
    * Creates new object only if is_new is true.
    * \param is_new Indicator if object is supposed to be created.
    */
   inline IPaddr_cpp(bool is_new);

   /**
    * \brief Alternative constructor. Assigns value from somewhere else.
    * \param ptr Pointer to IP that is not to be freed by destructor.
    */
   inline IPaddr_cpp(const ip_addr_t *ptr) : data(ptr) {new_object = false;};

   /**
    * \brief Destructor. Deletes address if new_object is set.
    */
   inline ~IPaddr_cpp();

   /**
    * \brief Implements methods for comparison operators
    * Operators: <, <=, >, >=, ==, !=
    * \param key2 Second operand.
    * \return Operation result
    */
   bool operator<(const IPaddr_cpp &key2) const;
   bool operator<=(const IPaddr_cpp &key2) const;
   bool operator>(const IPaddr_cpp &key2) const;
   bool operator>=(const IPaddr_cpp &key2) const;
   bool operator==(const IPaddr_cpp &key2) const;
   bool operator!=(const IPaddr_cpp &key2) const;

   /**
    * \brief Conversion from ip_addr_t* to std::string
    * \return IP address as a string.
    */
   std::string toString() const;

   /**
    * \brief Conversion from std::string to ip_addr_t*
    * \param String representation of IP.
    * \return True if conversion was successful.
    */
   bool fromString(std::string str);

   /**
    * \brief Returns currently stored IP.
    * \return IP address.
    */
   const ip_addr_t *get_IP();

   /**
    * \brief Sets IP address.
    * \param ip Value to be assigned to data.
    */
   void set_IP(const ip_addr_t *ip);

   /**
    * \brief Checks IP version/
    * \return True if stored data is IP of said version.
    */
   bool ip_isv4();
   bool ip_isv6();

   /**
    * \brief Returns IP version as an integer.
    * \return Returns 4 for IPv4, 6 for IPv6, 0 for invalid IP.
    */
   int get_version();

   /**
    * \brief Returns IP as vector of bytes (char).
    * \return Bytes representing IP (4 for IPv4, 16 for IPv6).
    */
   std::vector<char> get_bytes();
};

inline IPaddr_cpp::IPaddr_cpp()
{
   data = new ip_addr_t;
   new_object = true;
}

// If the object is used only for reading IP values from input
inline IPaddr_cpp::IPaddr_cpp(bool is_new)
{
   new_object = is_new;
   data = new_object ? new ip_addr_t : NULL;
}


inline IPaddr_cpp::~IPaddr_cpp()
{
    if (new_object) delete data;
}

// swap_bytes_
inline uint64_t swap_bytes(const uint64_t x)
{
   return
      ((x & 0x00000000000000ffLL) << 56) |
      ((x & 0x000000000000ff00LL) << 40) |
      ((x & 0x0000000000ff0000LL) << 24) |
      ((x & 0x00000000ff000000LL) << 8) |
      ((x & 0x000000ff00000000LL) >> 8) |
      ((x & 0x0000ff0000000000LL) >> 24) |
      ((x & 0x00ff000000000000LL) >> 40) |
      ((x & 0xff00000000000000LL) >> 56);
}

// Comparison operators
inline bool IPaddr_cpp::operator<(const IPaddr_cpp &key2) const {
   return ((swap_bytes(this->data->ui64[0]) < swap_bytes(key2.data->ui64[0])) ||
           ((swap_bytes(this->data->ui64[0]) == swap_bytes(key2.data->ui64[0])) &&
            (swap_bytes(this->data->ui64[1]) < swap_bytes(key2.data->ui64[1]))));
}

inline bool IPaddr_cpp::operator<=(const IPaddr_cpp &key2) const {
   return !(*this > key2);
}

inline bool IPaddr_cpp::operator>(const IPaddr_cpp &key2) const {
   return ((swap_bytes(this->data->ui64[0]) > swap_bytes(key2.data->ui64[0])) ||
           ((swap_bytes(this->data->ui64[0]) == swap_bytes(key2.data->ui64[0])) &&
            (swap_bytes(this->data->ui64[1]) > swap_bytes(key2.data->ui64[1]))));
}

inline bool IPaddr_cpp::operator>=(const IPaddr_cpp &key2) const {
   return !(*this < key2);
}

inline bool IPaddr_cpp::operator==(const IPaddr_cpp &key2) const {
   return ((this->data->ui64[0] == key2.data->ui64[0]) && (this->data->ui64[1] == key2.data->ui64[1]));
}
inline bool IPaddr_cpp::operator!=(const IPaddr_cpp &key2) const {
   return !(*this == key2);
}

//String conversions
inline std::string IPaddr_cpp::toString() const
{
   char buf[INET6_ADDRSTRLEN];
   ip_to_str(this->data, buf);
   return std::string(buf);
}

inline std::ostream& operator<<(std::ostream &os, const IPaddr_cpp &ip)
{
  return os << ip.toString();
}

// Creates new IP object from string
bool IPaddr_cpp::fromString(std::string str)
{
   ip_addr_t *addr_ptr = new ip_addr_t;
   if (!ip_from_str(str.c_str(), addr_ptr)) {
      return false; // Error - string is not a valid IP address
   } else {
      if (new_object) {
         delete data;
      } else {
         new_object = true;
      }
      data = addr_ptr;
      return true;
   }
}

// Get/set IP value
const ip_addr_t *IPaddr_cpp::get_IP()
{
   return data;
}

void IPaddr_cpp::set_IP(const ip_addr_t *ip)
{
   data = ip;
   return;
}

// Determine IP version
bool IPaddr_cpp::ip_isv4()
{
   return (data != NULL) && (ip_is4(data) == 1);
}

bool IPaddr_cpp::ip_isv6()
{
   return (data != NULL) && (ip_is6(data) == 1);
}

int IPaddr_cpp::get_version()
{
   if (data == NULL) {
      return 0;
   }
   return this->ip_isv4() ? 4 : 6;
}

// Get IP as bytes
std::vector<char> IPaddr_cpp::get_bytes()
{
   std::vector<char> ip;

   if (data == NULL) {
      return ip;
   }

   // Which bytes will be returned
   int min, max;
   if (this->ip_isv4()) {
      // IPv4 - 9th-12th byte (included)
      min = 8;
      max = 11;
   } else {
      min = 0;
      max = 15;
   }

   // Fill vector
   for (int i = min; i <= max; i++) {
      ip.push_back((char)(data->bytes[i]));
   }
   
   return ip;
}

#endif

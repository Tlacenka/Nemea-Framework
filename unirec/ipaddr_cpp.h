#ifndef _IPADDR_CPP_H
#define _IPADDR_CPP_H

#include "ipaddr.h"
#include <string>
#include <iostream>
#include <arpa/inet.h>     // INET6_ADDRSTRLEN value

class IPaddr_cpp {
   bool new_object;
public:
   const ip_addr_t *data;
   inline IPaddr_cpp();
   inline IPaddr_cpp(bool is_new);
   inline ~IPaddr_cpp();
   inline IPaddr_cpp(const ip_addr_t *ptr) : data(ptr) {new_object = false;};
   //inline ~IPaddr_cpp();
   bool operator<(const IPaddr_cpp &key2) const;
   bool operator<=(const IPaddr_cpp &key2) const;
   bool operator>(const IPaddr_cpp &key2) const;
   bool operator>=(const IPaddr_cpp &key2) const;
   bool operator==(const IPaddr_cpp &key2) const;
   bool operator!=(const IPaddr_cpp &key2) const;
   std::string toString() const;
   bool fromString(std::string str);
   void set_IP(ip_addr_t *ip);
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

void IPaddr_cpp::set_IP(ip_addr_t *ip)
{
   data = ip;
   return;
}

#endif

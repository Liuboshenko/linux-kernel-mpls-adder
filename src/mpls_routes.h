/**
 * @file mpls_routes.h
 * @brief Functions for managing MPLS routes in Linux via Netlink.
 *
 * This header defines functions for creating, swapping, and encapsulating MPLS routes
 * using network interfaces or next-hop IP addresses.
 */

 #ifndef MPLS_ROUTES_H
 #define MPLS_ROUTES_H
 
 #include <stdint.h>
 
 /**
  * @brief Creates an MPLS route using a specific interface.
  * @param interface Name of the network interface.
  * @param label MPLS label value (20 bits).
  * @param s_bit Bottom of Stack (BOS) bit (1 or 0).
  * @return 0 on success, -1 on failure.
  */
 int create_mpls_route_dev(const char *interface, uint32_t label, uint8_t s_bit);
 
 /**
  * @brief Creates an MPLS route via a next-hop IP address.
  * @param nexthop_ip Next-hop IP address.
  * @param label MPLS label value (20 bits).
  * @param s_bit Bottom of Stack (BOS) bit (1 or 0).
  * @return 0 on success, -1 on failure.
  */
 int create_mpls_route_nexthop(const char *nexthop_ip, uint32_t label, uint8_t s_bit);
 
 /**
  * @brief Creates an MPLS route that swaps an existing label via a specific interface.
  * @param interface Name of the network interface.
  * @param label Existing MPLS label (20 bits).
  * @param new_label New MPLS label to swap to (20 bits).
  * @param s_bit Bottom of Stack (BOS) bit (1 or 0).
  * @return 0 on success, -1 on failure.
  */
 int create_mpls_route_swap_dev(const char *interface, uint32_t label, uint32_t new_label, uint8_t s_bit);
 
 /**
  * @brief Creates an MPLS route that swaps an existing label via a next-hop IP address.
  * @param nexthop_ip Next-hop IP address.
  * @param label Existing MPLS label (20 bits).
  * @param new_label New MPLS label to swap to (20 bits).
  * @param s_bit Bottom of Stack (BOS) bit (1 or 0).
  * @return 0 on success, -1 on failure.
  */
 int create_mpls_route_swap_nexthop(const char *nexthop_ip, uint32_t label, uint32_t new_label, uint8_t s_bit);
 
 /**
 * @brief Creates an MPLS route with IP encapsulation via a specified interface.
 * 
 * This function configures an MPLS-encapsulated IP route by communicating 
 * directly with the Linux kernel using Netlink. The encapsulated packets 
 * will be forwarded via the specified network interface.
 * 
 * @param interface Name of the network interface to use for forwarding.
 * @param dst_ip Destination IP address for encapsulation.
 * @param mpls_label MPLS label to be pushed onto the packet.
 * @return 0 on success, -1 on failure.
 */
 int create_mpls_encap_route_dev(const char *interface, const char *dst_ip, uint32_t mpls_label);
 
 /**
 * @brief Creates an MPLS route with IP encapsulation via a gateway.
 * 
 * This function configures an MPLS-encapsulated IP route by communicating 
 * directly with the Linux kernel using Netlink. The encapsulated packets 
 * will be forwarded through a specified gateway.
 * 
 * @param dst_ip Destination IP address for encapsulation.
 * @param mpls_label MPLS label to be pushed onto the packet.
 * @param gateway_ip Gateway IP address for forwarding.
 * @return 0 on success, -1 on failure.
 */
 int create_mpls_encap_route_via(const char *dst_ip, uint32_t mpls_label, const char *gateway_ip);
 
 #endif // MPLS_ROUTES_H
 
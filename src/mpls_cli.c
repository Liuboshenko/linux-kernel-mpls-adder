/**
 * @file mpls_cli.c
 * @brief Command-line utility for managing MPLS routes via Netlink.
 *
 * This program allows users to add, modify, and encapsulate MPLS routes
 * in the Linux kernel using direct Netlink communication. It supports 
 * various MPLS operations such as label swapping, pushing labels onto 
 * IP packets, and specifying routes via interfaces or next-hop addresses.
 *
 * Usage:
 *  - mpls-cli add_for [label] dev [device_name]
 *  - mpls-cli add_for [label] next_hop [nexthop_ip]
 *  - mpls-cli add_for [label] swap_as [label_2] dev [device_name]
 *  - mpls-cli add_for [label] swap_as [label_2] next_hop [nexthop_ip]
 *  - mpls-cli add_for [dst_ip] push [label] next_hop [nexthop_ip]
 *  - mpls-cli add_for [dst_ip] push [label] dev [device_name]
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpls_routes.h" // Include header file for MPLS route management functions
#include "mpls_core.h"   // Include header file for core Netlink operations

/**
 * @brief Prints the usage instructions for the command-line tool.
 */
void print_usage() {
    printf("Usage:\n");
    printf("  mpls-cli add_for [label] dev [device_name]\n");
    printf("  mpls-cli add_for [label] next_hop [nexthop_ip]\n");
    printf("  mpls-cli add_for [label] swap_as [label_2] dev [device_name]\n");
    printf("  mpls-cli add_for [label] swap_as [label_2] next_hop [nexthop_ip]\n");
    printf("  mpls-cli add_for [dst_ip] push [label] next_hop [nexthop_ip]\n");
    printf("  mpls-cli add_for [dst_ip] push [label] dev [device_name]\n");
}

/**
 * @brief Main function for processing user commands and calling the corresponding MPLS route functions.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return EXIT_SUCCESS (0) on success, EXIT_FAILURE (1) on error.
 */
int main(int argc, char *argv[]) {
    // Check if the required minimum number of arguments is provided
    if (argc < 5) {
        printf("Error: Insufficient arguments.\n");
        print_usage();
        return EXIT_FAILURE;
    }

    // Validate the first argument (must be "add_for")
    if (strcmp(argv[1], "add_for") != 0) {
        printf("Error: Invalid command.\n");
        print_usage();
        return EXIT_FAILURE;
    }

    // Handle "add_for [label] dev [device_name]" command
    if (strcmp(argv[3], "dev") == 0 && argc == 5) {
        uint32_t label = atoi(argv[2]);
        const char *device = argv[4];
        return create_mpls_route_dev(device, label, 1);
    }
    
    // Handle "add_for [label] next_hop [nexthop_ip]" command
    if (strcmp(argv[3], "next_hop") == 0 && argc == 5) {
        uint32_t label = atoi(argv[2]);
        const char *nexthop_ip = argv[4];
        return create_mpls_route_nexthop(nexthop_ip, label, 1);
    }
    
    // Handle "add_for [label] swap_as [label_2] dev [device_name]" and
    // "add_for [label] swap_as [label_2] next_hop [nexthop_ip]" commands
    if (strcmp(argv[3], "swap_as") == 0 && argc == 7) {  
        uint32_t label = atoi(argv[2]);
        uint32_t new_label = atoi(argv[4]);
        if (strcmp(argv[5], "dev") == 0) {
            const char *device = argv[6]; 
            return create_mpls_route_swap_dev(device, label, new_label, 1);
        } else if (strcmp(argv[5], "next_hop") == 0) {
            const char *nexthop_ip = argv[6];
            return create_mpls_route_swap_nexthop(nexthop_ip, label, new_label, 1);
        }
    }
    
    // Handle "add_for [dst_ip] push [label] next_hop [nexthop_ip]" and
    // "add_for [dst_ip] push [label] dev [device_name]" commands
    if (strcmp(argv[3], "push") == 0 && argc == 7) { 
        uint32_t mpls_label = atoi(argv[4]);
        if (strcmp(argv[5], "next_hop") == 0) {
            const char *dst_ip = argv[2];
            const char *nexthop_ip = argv[6]; 
            return create_mpls_encap_route_via(dst_ip, mpls_label, nexthop_ip);
        } else if (strcmp(argv[5], "dev") == 0) {
            const char *dst_ip = argv[2];
            const char *device = argv[6];
            return create_mpls_encap_route_dev(device, dst_ip, mpls_label);
        }
    }
    
    // Print an error message if the command format is incorrect
    printf("Error: Invalid command format.\n");
    print_usage();
    return EXIT_FAILURE;
}

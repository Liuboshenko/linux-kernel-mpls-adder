# **USAGE.md - Guide to Using MPLS-CLI**

## **Introduction**
`mpls-cli` is a command-line utility for managing MPLS routes in Linux via **Netlink**. This tool allows adding, swapping, and encapsulating MPLS routes efficiently, without relying on `iproute2`.

---

## **1. Cloning the Repository**
Before starting, clone the repository and navigate to the project directory:

```sh
git clone <REPOSITORY_URL>
cd <REPOSITORY_NAME>
```

---

## **2. Installing Dependencies**
Before compiling, install the necessary libraries:

```sh
sudo apt update
sudo apt install build-essential libreadline-dev libncurses-dev
```

### **Dependency Explanation**
- `build-essential` – Installs `gcc`, `make`, and other necessary build tools.
- `libreadline-dev` – Enables command-line autocompletion.
- `libncurses-dev` – Provides terminal UI support (used in `readline`).

---

## **3. Compiling the Project**
To build the utility, run:

```sh
make
```

If the compilation is successful, the **`mpls-cli`** executable will be generated.

To clean up compiled files before recompilation:

```sh
make clean
```

---

## **4. Enabling Command Autocompletion**
To enhance usability, enable autocompletion for `mpls-cli`:

1. Make the script executable:
   ```sh
   chmod +x mpls-cli-completion.sh
   ```

2. Load the autocompletion script:
   ```sh
   source ./mpls-cli-completion.sh
   ```

3. To make autocompletion persistent across sessions, add it to `~/.bashrc`:
   ```sh
   echo "source /path/to/mpls-cli-completion.sh" >> ~/.bashrc
   source ~/.bashrc
   ```

Now, you can use `TAB` for command suggestions:
```sh
./mpls-cli [TAB]
```

---

## **5. Using `mpls-cli`**
After compilation, `mpls-cli` can be used to configure MPLS routes.

### **Basic Command Syntax**
```sh
mpls-cli add_for [LABEL/DEST_IP] [ACTION] [VALUE]
```

### **Available Commands**
| Command | Description |
|---------|------------|
| `add_for [label] dev [interface]` | Adds an MPLS route using a specific interface. |
| `add_for [label] next_hop [IP]` | Adds an MPLS route via a next-hop IP. |
| `add_for [label] swap_as [new_label] dev [interface]` | Swaps an MPLS label via an interface. |
| `add_for [label] swap_as [new_label] next_hop [IP]` | Swaps an MPLS label via a next-hop IP. |
| `add_for [dest_ip] push [label] next_hop [IP]` | Encapsulates an IP route into MPLS via a next-hop. |
| `add_for [dest_ip] push [label] dev [interface]` | Encapsulates an IP route into MPLS via an interface. |

---

## **6. Example Commands (Using the Test Stand)**

These examples demonstrate how to use `mpls-cli` to configure MPLS routes in the test stand environment.

#### **Adding an MPLS Route via Interface**
```sh
./mpls-cli add_for 100 dev veth_R1
```

#### **Adding an MPLS Route via Next-Hop**
```sh
./mpls-cli add_for 200 next_hop 10.1.1.2
```

#### **Swapping an MPLS Label via Interface**
```sh
./mpls-cli add_for 100 swap_as 300 dev veth_R2
```

#### **Swapping an MPLS Label via Next-Hop**
```sh
./mpls-cli add_for 100 swap_as 300 next_hop 10.2.2.2
```

#### **Encapsulating an IP Route into MPLS via Next-Hop**
```sh
./mpls-cli add_for 10.10.10.2 push 400 next_hop 10.1.1.1
```

#### **Encapsulating an IP Route into MPLS via Interface**
```sh
./mpls-cli add_for 10.10.10.2 push 400 dev veth_R1
```

---

## **7. Verifying Routes**
To verify the configured MPLS routes, use `iproute2`:

```sh
ip -f mpls route show
```

#### **Expected Output**
```
100 dev veth_R1
200 via inet 10.1.1.2
100 swap 300 dev veth_R2
100 swap 300 via inet 10.2.2.2
10.10.10.2 push 400 via inet 10.1.1.1
10.10.10.2 push 400 dev veth_R1
```

---

## **8. Inspecting MPLS Packet Forwarding**
To verify that MPLS labels are correctly applied and forwarded within the test stand, use `tcpdump`:

```sh
sudo ip netns exec Vhost_2 tcpdump -i veth2 -nn -v
```

#### **Example Output**
```
10:21:18 MPLS (label 111, ttl 64)
    IP (ttl 64, id 1616, proto ICMP, length 84)
    10.10.10.2 > 10.10.10.1: ICMP echo reply
```

This confirms that packets are forwarded with the expected MPLS labels through the network topology:

```
[lo(10.10.10.1/32)]                                                                 [lo(10.10.10.2/32)]  
        |                                                                                     |
    [Vhost_1]                                  MPLS-ROUTER                                [Vhost_2] 
        |                                            |                                        |
[veth1(10.1.1.2/24)] <--|--> [veth_R1(10.1.1.1/24)]  +  [veth_R2(10.2.2.1/24)] <--|--> [veth2(10.2.2.2)]
```

Using this structured approach, you can set up and verify MPLS routes within the isolated test stand environment.

## **9. Project Structure**
The project is structured as follows:

```
mpls-cli/
├── src
│   ├── mpls_cli.c        # CLI command handling
│   ├── mpls_core.c       # Netlink communication core
│   ├── mpls_routes.c     # MPLS route management functions
│   ├── mpls_core.h       # Header file for Netlink core
│   ├── mpls_routes.h     # Header file for MPLS route management
├── autocomplete
│   ├── mpls-cli-completion.sh # Bash autocompletion script
├── docs
│   ├── USAGE.md          # Usage guide
│   ├── DESIGN.md         # Design document
│   ├── HOW_TO_SETUP_MPLS_IN_LINUX.md # MPLS test stand setup
├── examples
│   ├── example_subnet.sh # Example configuration script
├── Makefile              # Build system
├── README.md             # Project overview
```

---

## **10. Handling Errors**
When adding routes, certain errors may occur. Below are common Netlink error codes and possible resolutions.

### **Invalid Argument (Code 22)**
**Cause:** One or more parameters are incorrect.  
**Solution:** 
- Ensure that the destination IP matches the prefix length (`/32` for single IP routes).
- Check that the MPLS label is within the valid range (`0-1048575`).
- Verify that all attributes (`dev`, `next_hop`, etc.) are correctly specified.

### **No Such Device**
**Cause:** The specified interface does not exist.  
**Solution:**
- Run `ip link show` to list available interfaces.
- Ensure the correct interface name (`veth_R1`, `veth_R2`, etc.) is used.

### **Network Unreachable**
**Cause:** The next-hop address is not reachable.  
**Solution:**
- Verify the next-hop is within the correct subnet.
- Ensure IP addresses are properly assigned to interfaces.

### **Permission Denied**
**Cause:** Insufficient privileges.  
**Solution:**
- Run `mpls-cli` with `sudo`.
- Ensure the user has sufficient permissions.

---

## **11. FAQ**

### **Why is my route not added?**
Ensure:
- The next-hop is reachable.
- The destination matches the prefix length.
- The kernel supports MPLS and the correct modules are loaded.

### **How do I check if MPLS is enabled?**
Run:
```sh
cat /proc/sys/net/mpls/platform_labels
```
If the output is greater than `0`, MPLS is enabled.

### **How do I remove an MPLS route?**
Currently, `mpls-cli` does not support route deletion. Use:
```sh
ip -f mpls route del [label]
```

---

## **13. Conclusion**
This guide provides a detailed overview of `mpls-cli`, including installation, usage, error handling, and troubleshooting. By leveraging **Netlink**, `mpls-cli` offers efficient MPLS route management.


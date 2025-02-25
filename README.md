# **MPLS-CLI: A Custom MPLS Route Management Tool**

## **Introduction**
`mpls-cli` is a lightweight command-line utility designed for managing MPLS (Multiprotocol Label Switching) routes in Linux. Unlike traditional methods that rely on `iproute2`, `mpls-cli` interacts directly with the Linux kernel via **Netlink**, providing efficient and precise MPLS route configuration.

This project is ideal for network engineers and developers who need to experiment with MPLS forwarding, label switching, and custom route configurations in an isolated test environment.

---

## **Features**
- Add, swap, and encapsulate MPLS routes using a simple CLI.
- Direct communication with the kernel via Netlink.
- Support for **interface-based** and **next-hop-based** MPLS routes.
- Easy integration with automated network testing environments.
- Built-in Bash autocompletion for faster command execution.

---

## **Getting Started**
### **1. Clone the Repository**
```sh
git clone <REPOSITORY_URL>
cd <REPOSITORY_NAME>
```

### **2. Install Dependencies**
Ensure you have the necessary dependencies installed:

```sh
sudo apt update
sudo apt install build-essential libreadline-dev libncurses-dev
```

### **3. Compile the Project**
Use `make` to build the project:

```sh
make
```

After a successful build, the `mpls-cli` binary will be available in the project directory.

To clean up compiled files:

```sh
make clean
```

### **4. Enable Command Autocompletion**
For convenience, enable Bash autocompletion:

```sh
chmod +x mpls-cli-completion.sh
source ./mpls-cli-completion.sh
```

To make it persistent across terminal sessions:

```sh
echo "source /path/to/mpls-cli-completion.sh" >> ~/.bashrc
source ~/.bashrc
```

---

## **Basic Commands**
### **Adding an MPLS Route via Interface**
```sh
./mpls-cli add_for 100 dev veth_R1
```

### **Adding an MPLS Route via Next-Hop**
```sh
./mpls-cli add_for 200 next_hop 10.1.1.2
```

### **Swapping an MPLS Label via Interface**
```sh
./mpls-cli add_for 100 swap_as 300 dev veth_R2
```

### **Swapping an MPLS Label via Next-Hop**
```sh
./mpls-cli add_for 100 swap_as 300 next_hop 10.2.2.2
```

### **Encapsulating an IP Route into MPLS via Next-Hop**
```sh
./mpls-cli add_for 10.10.10.2 push 400 next_hop 10.1.1.1
```

### **Encapsulating an IP Route into MPLS via Interface**
```sh
./mpls-cli add_for 10.10.10.2 push 400 dev veth_R1
```

---

## **Verifying MPLS Configuration**
To check the configured MPLS routes, use:
```sh
ip -f mpls route show
```

### **Expected Output**
```
100 dev veth_R1
200 via inet 10.1.1.2
100 swap 300 dev veth_R2
100 swap 300 via inet 10.2.2.2
10.10.10.2 push 400 via inet 10.1.1.1
10.10.10.2 push 400 dev veth_R1
```

To inspect packet forwarding, use `tcpdump`:
```sh
sudo ip netns exec Vhost_2 tcpdump -i veth2 -nn -v
```

---

## **Documentation**
For in-depth details, refer to the documentation:

- [**Installation & Usage Guide**](docs/USAGE.md) – Step-by-step instructions on compiling, installing, and using `mpls-cli`.
- [**Design Document**](docs/DESIGN.md) – Technical details on Netlink communication and the MPLS route structure.
- [**Setting Up an MPLS Test Stand**](docs/HOW_TO_SETUP_MPLS_IN_LINUX.md) – Instructions for creating an isolated MPLS test environment.
- [**Contributing**](docs/CONTRIBUTING.md) – Guidelines for contributing to the project.
- [**Technical Documentation**](docs/TECHNICAL_DOCUMENTATION_MPLS_ROUTE.md) – Technical Documentation: MPLS Route Management Implementation in Netlink

---

## **Troubleshooting & FAQ**
If you encounter issues while adding routes, here are some common error messages and solutions:

### **Invalid Argument (Code 22)**
- Ensure that the MPLS label is within the valid range (`0-1048575`).
- Verify that the prefix length for IP routes is correct (`/32` for single IPs).

### **No Such Device**
- Ensure the interface exists (`ip link show`).
- Use correct interface names (`veth_R1`, `veth_R2`, etc.).

### **Network Unreachable**
- Check if the next-hop is reachable.
- Ensure IP addresses are correctly assigned.

### **Permission Denied**
- Run `mpls-cli` with `sudo`.
- Ensure your user has the correct permissions.

For more error-handling scenarios, refer to the [Usage Guide](docs/USAGE.md#handling-errors).

---

## **Project Structure**
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
│   ├── CONTRIBUTING.md   # Contribution guidelines
|   ├── TECHNICAL_DOCUMENTATION_MPLS_ROUTE.md # Technical Documentation for mpls_routes.c
├── Makefile              # Build system
├── README.md             # Project overview
```

---

## **Contributing**
Contributions are welcome! If you would like to add new features, fix bugs, or improve documentation:
1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request.

For detailed contribution guidelines, see [CONTRIBUTING.md](docs/CONTRIBUTING.md).

---

## **License**
This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

## **Conclusion**
This README provides an overview of the `mpls-cli` project, from installation to usage and troubleshooting. For further details, refer to the linked documentation.

**Start using `mpls-cli` today and take full control over MPLS route management in Linux!**

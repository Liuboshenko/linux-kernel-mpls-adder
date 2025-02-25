#!/bin/bash

_mpls_cli_completion() {
    local cur prev words cword
    _get_comp_words_by_ref -n : cur prev words cword  # Retrieve current arguments

    # If the first argument (command)
    if [[ $cword -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "add_for" -- "$cur") )
        return
    fi

    # If the second argument (after "add_for")
    if [[ $cword -eq 2 && "${words[1]}" == "add_for" ]]; then
        COMPREPLY=()
        
        # Check for label (MPLS label must be within the range 0–1048575)
        if [[ "$cur" =~ ^[0-9]+$ ]] && ((cur >= 0 && cur <= 1048575)); then
            COMPREPLY=("$cur")  # Valid MPLS label
        # Check for an IP address (with optional subnet mask /XX)
        elif [[ "$cur" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+(/[0-9]{1,2})?$ ]]; then
            COMPREPLY=("$cur")  # Valid IP address or IP/subnet
        else
            COMPREPLY=("Enter an MPLS label (0-1048575) or an IP address (with or without a subnet mask)")
        fi
        return
    fi

    # Determine whether the first argument is a label or dst_ip
    local first_arg="${words[2]}"  # Third argument (label or dst_ip)

    # If the third argument is a label (number within 0-1048575), the next argument can be "dev", "next_hop", "swap_as"
    if [[ "$first_arg" =~ ^[0-9]+$ && "$first_arg" -ge 0 && "$first_arg" -le 1048575 && $cword -eq 3 ]]; then
        COMPREPLY=( $(compgen -W "dev next_hop swap_as" -- "$cur") )
        return
    fi

    # If the third argument is a dst_ip (IP or IP/subnet), the next argument must be "push"
    if [[ "$first_arg" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+(/[0-9]{1,2})?$ && $cword -eq 3 ]]; then
        COMPREPLY=( $(compgen -W "push" -- "$cur") )
        return
    fi

    # If "swap_as" was chosen, the next argument is expected to be [label_2]
    if [[ "${words[cword-1]}" == "swap_as" && $cword -eq 4 ]]; then
        COMPREPLY=( $(compgen -W "[Enter label_2]" -- "$cur") )
        return
    fi

    # If swap_as [label_2] is already entered, the next argument can be "dev" or "next_hop"
    if [[ "${words[cword-2]}" == "swap_as" && $cword -eq 5 ]]; then
        COMPREPLY=( $(compgen -W "dev next_hop" -- "$cur") )
        return
    fi

    # If "push" was chosen after dst_ip, expect a label [label]
    if [[ "${words[cword-1]}" == "push" && $cword -eq 4 ]]; then
        COMPREPLY=( $(compgen -W "[Enter label]" -- "$cur") )
        return
    fi

    # If "push [label]" is entered, expect "dev" or "next_hop"
    if [[ "${words[cword-2]}" == "push" && $cword -eq 5 ]]; then
        COMPREPLY=( $(compgen -W "dev next_hop" -- "$cur") )
        return
    fi

    # If "push", "label", "next_hop" are already entered → suggest entering an IP
    if [[ "${words[cword-3]}" == "push" && "${words[cword-1]}" == "next_hop" ]]; then
        COMPREPLY=( "[Here should be an IP address for next_hop.]" )
        return
    fi
}

# Enable autocompletion for ./mpls-cli
complete -F _mpls_cli_completion ./mpls-cli

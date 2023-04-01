#!/bin/bash

# Check that a command-line argument was provided
if [ $# -lt 2 ]; then
    echo "Usage: $0 <num_iterations> <plugin_name>"
    echo -e "Valid plugin_names:\naruba-6\nssh\nark-plugin"
    exit 1
fi

num_iterations=$1
plugin_name=$2

start_time=$(date +%s.%N)
for ((n = 0;n<$num_iterations;n++));do
        # Timestamp looks like: Mar 29 11:42:40
        timestamp=$(date +"%b %d %T")
        
        # Check the value of plugin_name and write to the appropriate log file
        if [ "$plugin_name" = "aruba-6" ]; then
        echo "$timestamp alienvault sapd[822]: <127038> <WARN> |AP HWISE-d8:c7:c8:cc:3f:7c@192.168.100.33 sapd| |ids-ap| AP(d8:c7:c8:43:f7:c0): Cleared Station Associated to Rogue AP: An AP is no longer detecting a client e8:92:a4:f9:ac:df associated to a rogue access point (BSSID 00:13:f7:ca:71:0f and SSID DWISEGEN on CHANNEL 7)" >> /var/log/demologs/aruba-6.log
        elif [ "$plugin_name" = "ssh" ]; then
        echo "$timestamp alienvault sshd[32297]: Failed password for invalid user root from 24.34.23.12 port 46766 ssh2" >> /var/log/auth.log
        elif [ "$plugin_name" = "ark-plugin" ]; then
        echo "$timestamp 172.16.16.225  $(date -u +"%Y-%m-%dT%H:%M:%S.%6NZ")#01115009 Query#011show tables" >> /var/log/ark-plugin.log
        fi
done

# Calculate stats
end_time=$(date +%s.%N)
elapsed_time=$(echo "$end_time - $start_time" | bc)
events_per_second=$(echo "scale=2; $num_iterations / $elapsed_time" | bc)

# Print stats
echo -e "\nNumber of iterations: $num_iterations"
echo "Elapsed time: $elapsed_time seconds"
echo -e "Events per second: $events_per_second\n"
